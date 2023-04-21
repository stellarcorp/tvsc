#pragma once

#include <cstdint>

#include "hal/output/output.h"
#include "hal/time/time.h"
#include "radio/packet_assembler.h"
#include "radio/packet_queue.h"
#include "radio/transceiver.h"

namespace tvsc::radio {

template <typename PacketT, size_t MTU, size_t MAX_TX_QUEUE_SIZE, size_t MAX_FRAGMENTS_PER_PACKET>
class TransceiverMonitor final {
 private:
  // Arbitrary. Need to make them configurable so that we can measure and pick values appropriate to
  // the environment.
  static constexpr size_t TX_ELEMENTS_AVAILABLE_THRESHOLD{4};
  static constexpr size_t TX_TIME_THRESHOLD_MS{50};
  static constexpr uint16_t TX_TIMEOUT_MS{50};

  HalfDuplexTransceiver<MTU>* radio_;

  PacketTxQueue<PacketT, MAX_TX_QUEUE_SIZE>* tx_queue_;
  PacketSink<PacketT, MAX_TX_QUEUE_SIZE> tx_queue_sink_;
  PacketAssembler<PacketT>* rx_queue_;
  bool cancel_requested_{false};

  struct RadioStatistics final {
    uint32_t packet_rx_count{};
    uint32_t packet_tx_count{};
    uint32_t dropped_packet_count{};
    uint32_t tx_failure_count{};
    uint64_t last_tx_time{};
    uint64_t last_statistics_publish_time{};
  };
  RadioStatistics statistics_{};
  uint32_t start_time_ms_{};

  bool should_transmit() const {
    return tx_queue_->elements_available() > TX_ELEMENTS_AVAILABLE_THRESHOLD ||
           tvsc::hal::time::time_millis() - statistics_.last_tx_time > TX_TIME_THRESHOLD_MS;
  }

  bool should_publish_statistics() const {
    return tvsc::hal::time::time_millis() - statistics_.last_statistics_publish_time > 1000;
  }

 public:
  TransceiverMonitor(HalfDuplexTransceiver<MTU>& radio,
                     PacketTxQueue<PacketT, MAX_TX_QUEUE_SIZE>& tx_queue,
                     PacketAssembler<PacketT>& rx_queue)
      : radio_(&radio),
        tx_queue_(&tx_queue),
        tx_queue_sink_(tx_queue_->create_sink()),
        rx_queue_(&rx_queue) {}

  void start() {
    cancel_requested_ = false;
    start_time_ms_ = tvsc::hal::time::time_millis();
    while (!cancel_requested_) {
      iterate();
    }
  }

  void cancel() { cancel_requested_ = true; }

  void iterate() {
    // Stay in receive mode as much as possible to avoid missing fragments.
    radio_->receive();

    // Receive a fragment, if one is available.
    if (radio_->has_fragment_available()) {
      Fragment<HalfDuplexTransceiver<MTU>::max_mtu()> fragment{};
      radio_->read_received_fragment(fragment);
      rx_queue_->add_fragment(fragment);
    }

    // Transmit any packets we have outstanding, if we decide we should transmit.
    if (!tx_queue_->empty()) {
      if (should_transmit()) {
        bool success{true};
        while (success && !tx_queue_->empty()) {
          const PacketT packet{tx_queue_sink_.peek()};

          EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
          encode(packet, fragments);

          for (size_t i = 0; i < fragments.num_fragments; ++i) {
            success = success && radio_->transmit_fragment(fragments.buffers[i], TX_TIMEOUT_MS);
          }
          if (success) {
            // This packet was transmitted successfully, so we can remove it from the queue.
            tx_queue_sink_.pop();
            statistics_.last_tx_time = tvsc::hal::time::time_millis();
          }
        }
        // Switch back to receive mode while we do other operations so that we don't miss fragments.
        radio_->receive();
      }
    }

    // Publish our statistics, if it is time.
    if (should_publish_statistics()) {
      statistics_.last_statistics_publish_time = tvsc::hal::time::time_millis();

      tvsc::hal::output::print("packet_rx_count: ");
      tvsc::hal::output::print(statistics_.packet_rx_count);
      tvsc::hal::output::print(", packet_tx_count: ");
      tvsc::hal::output::print(statistics_.packet_tx_count);
      tvsc::hal::output::print(", tx_queue_->elements_available(): ");
      tvsc::hal::output::print(tx_queue_->elements_available());
      tvsc::hal::output::print(", rx_queue_->num_incomplete_fragments(): ");
      tvsc::hal::output::print(rx_queue_->num_incomplete_fragments());
      tvsc::hal::output::print(", rx_queue_->num_outstanding_complete_packets(): ");
      tvsc::hal::output::print(rx_queue_->num_outstanding_complete_packets());
      tvsc::hal::output::print(", dropped_packet_count: ");
      tvsc::hal::output::print(statistics_.dropped_packet_count);
      tvsc::hal::output::print(", tx_failure_count: ");
      tvsc::hal::output::print(statistics_.tx_failure_count);
      tvsc::hal::output::print(", throughput: ");
      tvsc::hal::output::print(statistics_.packet_tx_count * 1000.f /
                               (tvsc::hal::time::time_millis() - start_time_ms_));
      tvsc::hal::output::print(" packets/sec");
      tvsc::hal::output::println();
    }
  }
};

}  // namespace tvsc::radio
