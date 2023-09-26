#pragma once

#include <cstdint>
#include <vector>

#include "comms/packet/packet_assembler.h"
#include "comms/packet/packet_queue.h"
#include "comms/radio/half_duplex_radio.h"
#include "comms/radio/telemetry_accumulator.h"
#include "comms/route/packet_router.h"
#include "comms/tdma/frame.h"
#include "comms/tdma/schedule.h"
#include "hal/time/clock.h"

namespace tvsc::comms::tdma {

template <typename PacketT, size_t MTU, size_t QUEUE_SIZE, size_t MAX_FRAGMENTS_PER_PACKET>
class TdmaTransceiverT final {
 private:
  using RadioT = tvsc::comms::radio::HalfDuplexRadio<MTU>;
  using FragmentT = tvsc::comms::radio::Fragment<MTU>;

  using PacketQueueT = tvsc::comms::packet::PacketTxQueue<PacketT, QUEUE_SIZE>;
  using FragmentSinkT =
      tvsc::comms::packet::FragmentSink<PacketT, QUEUE_SIZE, MTU, MAX_FRAGMENTS_PER_PACKET>;

  using PacketAssemblerT = tvsc::comms::packet::PacketAssembler<PacketT>;

  using PacketRouterT = tvsc::comms::route::PacketRouter<PacketT>;

  using TelemetryT = tvsc::comms::radio::TelemetryAccumulator;
  using ClockT = tvsc::hal::time::Clock;

  static constexpr uint32_t TX_TIMEOUT_US{3000};
  static constexpr uint64_t RSSI_MEASUREMENT_INTERVAL_US{10000};

  void maybe_measure_rssi(uint64_t current_time_us) {
    if (current_time_us - last_rssi_measurement_time_us_ > RSSI_MEASUREMENT_INTERVAL_US) {
      if (schedule_.time_slot_duration_remaining_us() > radio_->rssi_measurement_time_us()) {
        last_rssi_measurement_time_us_ = current_time_us;
        telemetry_->set_rssi_dbm(radio_->read_rssi_dbm());
      }
    }
  }

  void process_rx(uint64_t /*current_time_us*/) {
    // See if the radio has any fragments to receive.
    if (radio_->has_fragment_available()) {
      FragmentT fragment{};
      radio_->read_received_fragment(fragment);

      telemetry_->increment_fragments_received();

      // Detect if we have dropped any fragments/packets.
      if (fragment.sequence_number() != previous_sequence_number_ + 1 &&
          previous_sequence_number_ != 0) {
        telemetry_->increment_fragments_dropped();
        tvsc::hal::output::print("Dropped fragments. fragment.sequence_number: ");
        tvsc::hal::output::print(fragment.sequence_number());
        tvsc::hal::output::print(", previous_sequence_number: ");
        tvsc::hal::output::print(previous_sequence_number_);
        tvsc::hal::output::println();
      }

      previous_sequence_number_ = fragment.sequence_number();

      tvsc::hal::output::print("From sender: ");
      tvsc::hal::output::print(fragment.sender_id());
      tvsc::hal::output::print(", sequence: ");
      tvsc::hal::output::print(fragment.sequence_number());
      tvsc::hal::output::print(", payload_size: ");
      tvsc::hal::output::println(fragment.payload_size());

      packet_assembler_.add_fragment(std::move(fragment));
    }
  }

  void process_tx(uint64_t current_time_us) {
    // Return if there are any issues keeping us from transmitting.
    if (radio_->is_transmitting_fragment()) {
      if (current_time_us - fragment_tx_initiated_us_ < TX_TIMEOUT_US) {
        tvsc::hal::output::println(
            "TdmaTransceiver::process_tx() -- Pausing until existing TX complete.");
        return;
      } else {
        // Abandon transmission and try again.
        radio_->set_standby_mode();
        have_fragment_in_transmission_ = false;
        telemetry_->increment_transmit_errors();
        tvsc::hal::output::println("TdmaTransceiver::process_tx() -- TX timed out.");
        return;
      }
    }

    // Mark a previous transmission, if any, as complete and load the next fragment.
    if (have_fragment_in_transmission_) {
      have_fragment_in_transmission_ = false;
      fragment_sink_.next_fragment();
      telemetry_->increment_fragments_transmitted();
    }

    if (radio_->channel_activity_detected()) {
      // Should not happen in TDMA, but...
      tvsc::hal::output::println(
          "TdmaTransceiver::process_tx() -- Pausing due to channel activity.");
      return;
    }

    // We are ready to transmit.

    // Load the next fragment to transmit, if any.
    if (!fragment_sink_.has_more_fragments()) {
      if (fragment_sink_.have_packet()) {
        fragment_sink_.pop_packet();
      }
      if (!packet_queue_.empty()) {
        fragment_sink_.encode_next_packet();
      }
    }

    // We have fragments to transmit.
    if (fragment_sink_.has_more_fragments() &&  //
        schedule_.time_slot_duration_remaining_us() > radio_->fragment_transmit_time_us()) {
      const FragmentT& fragment = fragment_sink_.fragment();

      bool result;
      result = radio_->transmit_fragment(fragment);
      if (result) {
        tvsc::hal::output::println(
            "TdmaTransceiver::process_tx() -- Fragment transmission initiated.");
        fragment_tx_initiated_us_ = current_time_us;
        have_fragment_in_transmission_ = true;
      } else {
        tvsc::hal::output::println(
            "TdmaTransceiver::process_tx() -- Failed to start transmission.");
        telemetry_->increment_transmit_errors();
      }
    }
  }

  void route_packet(PacketT packet) {
    for (auto* router : routers_) {
      if (router->route(packet)) {
        break;
      }
    }
  }

  RadioT* radio_;
  TelemetryT* telemetry_;
  ClockT* clock_;

  Schedule schedule_{*clock_};

  PacketQueueT packet_queue_{};
  FragmentSinkT fragment_sink_{packet_queue_};

  PacketAssemblerT packet_assembler_{};

  std::vector<PacketRouterT*> routers_{};

  uint32_t previous_sequence_number_{};
  uint16_t next_telemetry_metric_to_report_{0};
  uint16_t next_telemetry_sequence_number_{0};

  uint64_t last_telemetry_report_time_us_{};
  uint64_t last_rssi_measurement_time_us_{};

  uint64_t fragment_tx_initiated_us_{};
  bool have_fragment_in_transmission_{false};

 public:
  TdmaTransceiverT(RadioT& radio, TelemetryT& telemetry, ClockT& clock)
      : radio_(&radio), telemetry_(&telemetry), clock_(&clock) {}

  void iterate() {
    const uint64_t current_time_us{clock_->current_time_micros()};

    if (schedule_.should_receive()) {
      radio_->set_receive_mode();
      process_rx(current_time_us);
    } else if (schedule_.can_transmit()) {
      process_tx(current_time_us);

      // Nothing to transmit, or couldn't transmit.
      if (!radio_->is_transmitting_fragment()) {
        // Since we own the time slot, this RSSI measurement should be background noise.
        maybe_measure_rssi(current_time_us);
      }
    } else {
      // Do not need to receive and cannot transmit.
      // Note that we don't know if this measurement will be background noise or another transmitter
      // in the cell.
      maybe_measure_rssi(current_time_us);
    }

    while (packet_assembler_.has_complete_packets()) {
      route_packet(packet_assembler_.consume_packet());
    }

    telemetry_->set_transmit_queue_size(packet_queue_.size());
  }

  void set_frame(const Frame& frame) { schedule_.set_frame(frame); }
  void set_id(uint64_t id) { schedule_.set_id(id); }

  void push_immediate_priority(const PacketT& packet) {
    packet_queue_.push_immediate_priority(packet);
  }

  void push_control_priority(const PacketT& packet) { packet_queue_.push_control_priority(packet); }

  void push_normal_priority(const PacketT& packet) { packet_queue_.push_normal_priority(packet); }

  void push_low_priority(const PacketT& packet) { packet_queue_.push_low_priority(packet); }

  size_t transmit_queue_size() const { return packet_queue_.size(); }

  void add_router(PacketRouterT& router) { routers_.emplace_back(&router); }

  const Schedule& schedule() const { return schedule_; }
};

}  // namespace tvsc::comms::tdma
