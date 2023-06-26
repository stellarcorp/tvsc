#pragma once

#include <atomic>
#include <cstdint>
#include <functional>

#include "glog/logging.h"
#include "hal/output/output.h"
#include "hal/time/time.h"
#include "radio/packet_assembler.h"
#include "radio/packet_queue.h"
#include "radio/transceiver.h"
#include "radio/transceiver_utilities.h"

namespace tvsc::radio {

template <typename PacketT, size_t MTU, size_t MAX_TX_QUEUE_SIZE, size_t MAX_FRAGMENTS_PER_PACKET>
class TransceiverMonitor final {
 private:
  // Arbitrary. Need to make them configurable so that we can measure and pick values appropriate to
  // the environment.
  static constexpr size_t TX_ELEMENTS_AVAILABLE_THRESHOLD{4};
  static constexpr size_t TX_TIME_THRESHOLD_MS{50};
  static constexpr uint16_t TX_TIMEOUT_MS{200};

  HalfDuplexTransceiver<MTU>* radio_;

  PacketTxQueue<PacketT, MAX_TX_QUEUE_SIZE>* tx_queue_;
  PacketSink<PacketT, MAX_TX_QUEUE_SIZE> tx_queue_sink_;
  PacketAssembler<PacketT>* rx_queue_;
  std::function<void(const Packet& packet)> notify_fn_;

  std::atomic<bool> is_running_{false};
  std::atomic<bool> cancel_requested_{false};

  struct RadioStatistics final {
    uint32_t packet_rx_count{0};
    uint32_t packet_tx_count{0};
    uint32_t dropped_packet_count{0};
    uint32_t tx_failure_count{0};
    uint64_t last_tx_time{0};
    uint64_t last_statistics_publish_time{0};
  };
  RadioStatistics statistics_{};
  uint32_t start_time_ms_{};

  bool should_transmit() const {
    return tx_queue_->elements_available() > TX_ELEMENTS_AVAILABLE_THRESHOLD ||
           tvsc::hal::time::time_millis() - statistics_.last_tx_time > TX_TIME_THRESHOLD_MS;
  }

  bool should_publish_statistics() const {
    uint64_t time_since_last_publish_ms =
        tvsc::hal::time::time_millis() - statistics_.last_statistics_publish_time;
    return time_since_last_publish_ms > 5000;
  }

 public:
  TransceiverMonitor(HalfDuplexTransceiver<MTU>& radio,
                     PacketTxQueue<PacketT, MAX_TX_QUEUE_SIZE>& tx_queue,
                     PacketAssembler<PacketT>& rx_queue,
                     std::function<void(const Packet& packet)> notify_fn)
      : radio_(&radio),
        tx_queue_(&tx_queue),
        tx_queue_sink_(tx_queue_->create_sink()),
        rx_queue_(&rx_queue),
        notify_fn_(std::move(notify_fn)) {
    radio_->reset();
  }

  ~TransceiverMonitor() {
    DLOG(INFO) << "TransceiverMonitor::~TransceiverMonitor()";
    cancel();
    static constexpr uint32_t max_delay_ms{500};
    uint32_t total_delay_ms{0};
    while (is_running_ && total_delay_ms < max_delay_ms) {
      total_delay_ms += 5;
      tvsc::hal::time::delay_ms(5);
    }
    if (is_running_) {
      LOG(FATAL) << "TransceiverMonitor::~TransceiverMonitor() -- Background thread still running. "
                    "This will cause a crash in the future. Aborting.";
    }
  }

  void start() {
    cancel_requested_ = false;
    is_running_ = true;
    start_time_ms_ = tvsc::hal::time::time_millis();
    while (!cancel_requested_) {
      iterate();
    }
    DLOG(INFO) << "TransceiverMonitor::start() -- cancel_requested_ is true";
    is_running_ = false;
  }

  void cancel() {
    DLOG(INFO) << "TransceiverMonitor::cancel()";
    cancel_requested_ = true;
  }

  void iterate() {
    // Stay in receive mode as much as possible to avoid missing fragments.
    radio_->set_receive_mode();

    // Receive a fragment, if one is available.
    if (radio_->has_fragment_available()) {
      // if (radio_->wait_fragment_available(50)) {
      Fragment<HalfDuplexTransceiver<MTU>::max_mtu()> fragment{};
      radio_->read_received_fragment(fragment);
      rx_queue_->add_fragment(fragment);
    }

    if (rx_queue_->has_complete_packets()) {
      notify_fn_(rx_queue_->consume_packet());
    }

    // Transmit any packets we have outstanding, if we decide we should transmit.
    if (!tx_queue_->empty()) {
      if (should_transmit()) {
        bool success{true};
        const uint64_t transmission_cycle_start_time{tvsc::hal::time::time_millis()};
        while (success && !tx_queue_->empty()) {
          const uint64_t packet_transmission_start_time{tvsc::hal::time::time_millis()};
          const PacketT packet{tx_queue_sink_.peek()};

          EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
          encode(packet, fragments);

          for (size_t i = 0; i < fragments.num_fragments; ++i) {
            success = success && radio_->transmit_fragment(fragments.buffers[i], TX_TIMEOUT_MS);
            if (!success) {
              LOG(INFO) << "TransceiverMonitor::iterate() -- transmit_fragment() -- failed.";
            }
            // success = success && radio_->wait_fragment_transmitted(TX_TIMEOUT_MS);
            // TODO(james): We aren't reliably getting the interrupt that says the packet was
            // transmitted. Ignoring the return value here for the time being.
            success = success && block_until_transmission_complete(*radio_, TX_TIMEOUT_MS);
            if (!success) {
              LOG(INFO) << "TransceiverMonitor::iterate() -- block_until_transmission_complete() "
                           "-- failed.";
            }
          }
          if (success) {
            // This packet was transmitted successfully, so we can remove it from the queue.
            tx_queue_sink_.pop();
            statistics_.last_tx_time = tvsc::hal::time::time_millis();
            LOG(INFO)
                << "TransceiverMonitor::iterate() -- Transmit successful. Transmission time (ms): "
                << (statistics_.last_tx_time - packet_transmission_start_time);
          } else {
            LOG(INFO) << "TransceiverMonitor::iterate() -- transmit failed.";
          }
        }
        // Switch back to receive mode while we do other operations so that we don't miss
        // fragments.
        radio_->set_receive_mode();

        LOG(INFO) << "TransceiverMonitor::iterate() -- switched back to receive mode. Full "
                     "transmission cycle time (ms): "
                  << (tvsc::hal::time::time_millis() - transmission_cycle_start_time);
      }
    }
  }
};

}  // namespace tvsc::radio
