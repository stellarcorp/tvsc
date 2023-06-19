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
    LOG(WARNING) << "TransceiverMonitor::should_transmit()";
    LOG(WARNING) << "TransceiverMonitor::should_transmit() -- tx_queue_->elements_available(): "
                 << tx_queue_->elements_available();
    LOG(WARNING) << "TransceiverMonitor::should_transmit() -- tvsc::hal::time::time_millis() - "
                    "statistics_.last_tx_time: "
                 << (tvsc::hal::time::time_millis() - statistics_.last_tx_time);
    return tx_queue_->elements_available() > TX_ELEMENTS_AVAILABLE_THRESHOLD ||
           tvsc::hal::time::time_millis() - statistics_.last_tx_time > TX_TIME_THRESHOLD_MS;
  }

  bool should_publish_statistics() const {
    return tvsc::hal::time::time_millis() - statistics_.last_statistics_publish_time > 1000;
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
    bool did_something{false};
    // Stay in receive mode as much as possible to avoid missing fragments.
    radio_->set_receive_mode();

    // Receive a fragment, if one is available.
    if (radio_->has_fragment_available()) {
      LOG(INFO) << "TransceiverMonitor::iterate() -- radio has RX fragment available.";
      Fragment<HalfDuplexTransceiver<MTU>::max_mtu()> fragment{};
      radio_->read_received_fragment(fragment);
      rx_queue_->add_fragment(fragment);
      LOG(INFO) << "TransceiverMonitor::iterate() -- fragment added to RX queue.";

      did_something = true;
    }

    if (rx_queue_->has_complete_packets()) {
      LOG(INFO) << "TransceiverMonitor::iterate() -- radio has complete RX packet available.";
      notify_fn_(rx_queue_->consume_packet());

      did_something = true;
    }

    // Transmit any packets we have outstanding, if we decide we should transmit.
    if (!tx_queue_->empty()) {
      LOG(INFO) << "TransceiverMonitor::iterate() -- tx_queue has packets available.";
      if (should_transmit()) {
        LOG(INFO) << "TransceiverMonitor::iterate() -- transmitting.";
        bool success{true};
        while (success && !tx_queue_->empty()) {
          const PacketT packet{tx_queue_sink_.peek()};

          EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
          encode(packet, fragments);

          for (size_t i = 0; i < fragments.num_fragments; ++i) {
            LOG(INFO) << "TransceiverMonitor::iterate() -- transmit_fragment().";
            success = success && radio_->transmit_fragment(fragments.buffers[i], TX_TIMEOUT_MS);
            if (success) {
              LOG(INFO) << "TransceiverMonitor::iterate() -- transmit_fragment() -- success.";
            } else {
              LOG(INFO) << "TransceiverMonitor::iterate() -- transmit_fragment() -- failed.";
            }
            LOG(INFO) << "TransceiverMonitor::iterate() -- wait_fragment_transmitted().";
            success = success && radio_->wait_fragment_transmitted(TX_TIMEOUT_MS);
            if (success) {
              LOG(INFO)
                  << "TransceiverMonitor::iterate() -- wait_fragment_transmitted() -- success.";
            } else {
              LOG(INFO)
                  << "TransceiverMonitor::iterate() -- wait_fragment_transmitted() -- failed.";
            }
          }
          if (success) {
            // This packet was transmitted successfully, so we can remove it from the queue.
            LOG(INFO)
                << "TransceiverMonitor::iterate() -- transmit successful. Popping off of TX queue.";
            tx_queue_sink_.pop();
            statistics_.last_tx_time = tvsc::hal::time::time_millis();
          } else {
            LOG(INFO) << "TransceiverMonitor::iterate() -- transmit failed.";
          }
        }
        // Switch back to receive mode while we do other operations so that we don't miss fragments.
        LOG(INFO) << "TransceiverMonitor::iterate() -- switching back to receive mode.";
        radio_->set_receive_mode();

        did_something = true;
      }
    }

    // Publish our statistics, if it is time.
    if (should_publish_statistics()) {
      statistics_.last_statistics_publish_time = tvsc::hal::time::time_millis();

      LOG(INFO) << "packet_rx_count: " << statistics_.packet_rx_count
                << ", packet_tx_count: " << statistics_.packet_tx_count
                << ", tx_queue_->elements_available(): " << tx_queue_->elements_available()
                << ", rx_queue_->num_incomplete_fragments(): "
                << rx_queue_->num_incomplete_fragments()
                << ", rx_queue_->num_outstanding_complete_packets(): "
                << rx_queue_->num_outstanding_complete_packets()
                << ", dropped_packet_count: " << statistics_.dropped_packet_count
                << ", tx_failure_count: " << statistics_.tx_failure_count
                << ", throughput: " << statistics_.packet_tx_count * 1000.f << " packets/sec";
    }

    if (!did_something) {
      tvsc::hal::time::delay_ms(1);
    }
  }
};

}  // namespace tvsc::radio
