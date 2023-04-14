#pragma once

#include <stdexcept>

#include "buffer/ring_buffer.h"
#include "random/random.h"

namespace tvsc::radio {

template <typename PacketT, size_t NUM_PACKETS>
class PacketTxQueue;

template <typename PacketT, size_t NUM_PACKETS>
class PacketSink final {
 public:
  using PacketTxQueueT = PacketTxQueue<PacketT, NUM_PACKETS>;

  using DataAvailableCallback = std::function<void(PacketTxQueueT&)>;

 private:
  struct PeekResponse {
    const PacketT* packet{nullptr};
    const void* queue{nullptr};
  };

  PacketTxQueueT* packet_queue_{nullptr};
  PeekResponse last_peek_{};

  PacketSink(PacketTxQueueT& queue) : packet_queue_(&queue) {}

  friend PacketTxQueueT;

 public:
  PacketTxQueueT& packet_queue() { return *packet_queue_; }

  /**
   * Peek at the current packet. Subsequent calls to peek() are not guaranteed to return the same
   * packet.
   *
   * The expected idiom here is to peek() at the current packet, attempt to process it, and if the
   * packet is processed successfully, then call pop() to remove it. If there is an error in the
   * processing, don't call pop().
   *
   * The packet queue implements a form of fair queuing. The algorithm to choose which packet to
   * send is implemented in the peek() call.
   */
  const PacketT& peek() {
    last_peek_ = packet_queue_->peek();
    if (last_peek_.queue != nullptr) {
      return *last_peek_.packet;
    } else {
      throw std::logic_error("No packet available in PacketSink::peek()");
    }
  }

  /**
   * Removes a packet from the packet queue, dropping it completely. This is typically done after
   * the packet is peek'd from the queue and is processed successfully.
   */
  void pop() {
    packet_queue_->pop(last_peek_);
    last_peek_.packet = nullptr;
    last_peek_.queue = nullptr;
  }

  bool empty() const { return packet_queue_->empty(); }

  void set_data_available_callback(DataAvailableCallback callback) {
    if (callback) {
      packet_queue_->set_data_available_callback(
          [this, callback](PacketTxQueueT& queue) { callback(*this); });
    } else {
      packet_queue_->set_data_available_callback(std::function<void(PacketTxQueueT&)>{});
    }
  }
};

/**
 * A queue structure for scheduling the sending of packets.
 *
 * The PacketTxQueue implements a variation on generalized processor sharing
 * (https://en.wikipedia.org/wiki/Generalized_processor_sharing), with the exception that it
 * provides for an immediate priority that always supercedes all other priorities. The
 * PacketTxQueue provides for four classes of service:
 *
 * - immediate: Used for communications whose timeliness affects safety or viability of the
 * vehicle. These packets override all other packets.
 *
 * - control: Used for communications to control the vehicle, communications channels, etc. as
 * well as high priority telemetry data.
 *
 * - normal: Used for communications with no special priority. Most packets should have this
 * priority.
 *
 * - low: Used for communications that should be considered optional. This class mainly includes
 * low priority telemetry data.
 */
template <typename PacketT, size_t NUM_PACKETS>
class PacketTxQueue final {
 private:
  using DataAvailableCallback = std::function<void(PacketTxQueue&)>;

  using RingBufferT = tvsc::buffer::RingBuffer<PacketT, 1, NUM_PACKETS, false>;

  using WeightT = unsigned int;
  static constexpr WeightT CONTROL_PRIORITY_WEIGHT{100};
  static constexpr WeightT NORMAL_PRIORITY_WEIGHT{10};
  static constexpr WeightT LOW_PRIORITY_WEIGHT{1};
  static constexpr WeightT WEIGHT_TOTAL{CONTROL_PRIORITY_WEIGHT + NORMAL_PRIORITY_WEIGHT +
                                        LOW_PRIORITY_WEIGHT};

  static bool select_control_priority_queue(WeightT* value) {
    if (*value < CONTROL_PRIORITY_WEIGHT) {
      return true;
    } else {
      *value -= CONTROL_PRIORITY_WEIGHT;
      return false;
    }
  }

  static bool select_normal_priority_queue(WeightT* value) {
    if (*value < NORMAL_PRIORITY_WEIGHT) {
      return true;
    } else {
      *value -= NORMAL_PRIORITY_WEIGHT;
      return false;
    }
  }

  static bool select_low_priority_queue(WeightT* value) {
    if (*value < LOW_PRIORITY_WEIGHT) {
      return true;
    } else {
      *value -= LOW_PRIORITY_WEIGHT;
      return false;
    }
  }

  RingBufferT immediate_priority_{};
  RingBufferT control_priority_{};
  RingBufferT normal_priority_{};
  RingBufferT low_priority_{};

  typename PacketSink<PacketT, NUM_PACKETS>::PeekResponse peek() const {
    typename PacketSink<PacketT, NUM_PACKETS>::PeekResponse result{};
    bool packet_selected{false};
    if (!immediate_priority_.empty()) {
      packet_selected = immediate_priority_.peek(&result.packet);
      if (packet_selected) {
        result.queue = const_cast<void*>(reinterpret_cast<const void*>(&immediate_priority_));
      }
    }
    if (!packet_selected) {
      WeightT random_value = tvsc::random::generate_random_value<WeightT>(0, WEIGHT_TOTAL);
      if (!packet_selected && select_control_priority_queue(&random_value)) {
        packet_selected = control_priority_.peek(&result.packet);
        if (packet_selected) {
          result.queue = const_cast<void*>(reinterpret_cast<const void*>(&control_priority_));
        } else {
          // No packets at this priority. Try the next lower priority packets.
          random_value = NORMAL_PRIORITY_WEIGHT - 1;
        }
      }
      if (!packet_selected && select_normal_priority_queue(&random_value)) {
        packet_selected = normal_priority_.peek(&result.packet);
        if (packet_selected) {
          result.queue = const_cast<void*>(reinterpret_cast<const void*>(&normal_priority_));
        } else {
          // No packets at this priority. Try the next lower priority packets.
          random_value = LOW_PRIORITY_WEIGHT - 1;
        }
      }
      if (!packet_selected && select_low_priority_queue(&random_value)) {
        packet_selected = low_priority_.peek(&result.packet);
        if (packet_selected) {
          result.queue = const_cast<void*>(reinterpret_cast<const void*>(&low_priority_));
        }
      }
    }
    return result;
  }

  void pop(const typename PacketSink<PacketT, NUM_PACKETS>::PeekResponse& previous_peek) {
    if (previous_peek.queue == &immediate_priority_) {
      immediate_priority_.pop();
    } else if (previous_peek.queue == &control_priority_) {
      control_priority_.pop();
    } else if (previous_peek.queue == &normal_priority_) {
      normal_priority_.pop();
    } else if (previous_peek.queue == &low_priority_) {
      low_priority_.pop();
    } else {
      throw std::logic_error("This should not happen.");
    }
  }

  friend class PacketSink<PacketT, NUM_PACKETS>;

  void set_data_available_callback(DataAvailableCallback callback) {
    if (callback) {
      immediate_priority_.set_data_available_callback(
          [this, callback](RingBufferT& /*ring*/) { callback(*this); });
      control_priority_.set_data_available_callback(
          [this, callback](RingBufferT& /*ring*/) { callback(*this); });
      normal_priority_.set_data_available_callback(
          [this, callback](RingBufferT& /*ring*/) { callback(*this); });
      low_priority_.set_data_available_callback(
          [this, callback](RingBufferT& /*ring*/) { callback(*this); });
    } else {
      immediate_priority_.set_data_available_callback(
          typename RingBufferT::DataAvailableCallback{});
      control_priority_.set_data_available_callback(typename RingBufferT::DataAvailableCallback{});
      normal_priority_.set_data_available_callback(typename RingBufferT::DataAvailableCallback{});
      low_priority_.set_data_available_callback(typename RingBufferT::DataAvailableCallback{});
    }
  }

 public:
  void push_immediate(const PacketT& packet) { immediate_priority_.supply(packet); }

  void push_control(const PacketT& packet) { control_priority_.supply(packet); }

  void push_normal(const PacketT& packet) { normal_priority_.supply(packet); }

  void push_low_priority(const PacketT& packet) { low_priority_.supply(packet); }

  bool empty() const {
    return immediate_priority_.empty() && control_priority_.empty() && normal_priority_.empty() &&
           low_priority_.empty();
  }

  PacketSink<PacketT, NUM_PACKETS> create_sink() { return PacketSink<PacketT, NUM_PACKETS>{*this}; }
};

/**
 * PacketRxQueue is just a RingBuffer configured to prioritize new packets and dropping old
 * packets if the buffer is full.
 */
template <typename PacketT, size_t NUM_PACKETS>
using PacketRxQueue =
    tvsc::buffer::RingBuffer<PacketT, 1, NUM_PACKETS, /*PRIORITIZE_OLD_ELEMENTS*/ false>;

}  // namespace tvsc::radio
