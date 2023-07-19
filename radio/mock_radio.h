#pragma once

#include <atomic>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <vector>

#include "glog/logging.h"
#include "hal/time/clock.h"
#include "radio/radio_module.h"

namespace tvsc::radio {

template <size_t MTU>
class MockRadioT final : public HalfDuplexRadio<MTU> {
 public:
  // Amount of time in milliseconds required to transmit a fragment, including ramping up and down
  // the amplifiers, DSP operations and DAC timing. The radio must stay continuously in TX
  // mode during this time.
  static constexpr uint16_t FRAGMENT_TRANSMIT_TIME_MS{2};

 private:
  mutable std::mutex mutex_{};

  tvsc::hal::time::Clock* clock_{&tvsc::hal::time::default_clock()};

  // Timestamp of the last time a fragment was received. This should just increment in
  // fragment_receive_interval_ms increments and is used to figure out when the next fragment should
  // be received.
  uint64_t last_receive_time_ms_{};

  // Timestamp of the last time the radio switched into TX mode. In order to transmit a
  // fragment, we must be in TX mode for at least FRAGMENT_TRANSMIT_TIME_MS, else that fragment is
  // marked as corrupted.
  mutable uint64_t last_switch_to_tx_mode_ms_{0};

  // Collection of fragments that will be "received" when read_received_fragment() is called. These
  // fragments are indexed by a timestamp of when the fragment should be received. This timestamp is
  // in milliseconds according to the Clock::current_time_millis() method, or the MockClock override
  // of the same method.
  mutable std::map<uint64_t, Fragment<MTU>> rx_fragments_{};

  // Collection of fragments that have been sent via call to transmit_fragment().
  mutable std::vector<Fragment<MTU>> sent_fragments_{};

  enum class Mode {
    STANDBY,
    RX,
    TX,
  };
  Mode current_mode_{Mode::STANDBY};

  // Count of fragments dropped due to infrequent reading.
  mutable size_t count_dropped_fragments_{0};

  // Count of packets corrupted due to too frequent of transmissions or corrupted because we
  // switched out of TX mode before it was transmitted. If transmit_fragment(), set_receive_mode(),
  // set_standby_mode(), read_received_fragment() or possibly other functions are called before
  // FRAGMENT_TRANSMIT_TIME_MS after the previous call to transmit_fragment(), the previous fragment
  // is corrupted. Basically, half-duplex radios need time to finish their transmissions before they
  // can reconfigured for other tasks.
  mutable size_t count_corrupted_fragments_{0};

  // Most radios we are working with only have storage for a single fragment. This may be the
  // fragment that was most recently received, or the fragment being transmitted. That means that
  // switching modes can drop/corrupt fragments.
  mutable Fragment<MTU> buffered_fragment_{};

  mutable bool have_fragment_for_tx_{false};

  void handle_fragment_transmit() const {
    if (in_tx_mode()) {
      const std::lock_guard<std::mutex> lock(mutex_);
      if (have_fragment_for_tx_) {
        if (clock_->current_time_millis() - last_switch_to_tx_mode_ms_ >=
            FRAGMENT_TRANSMIT_TIME_MS) {
          sent_fragments_.push_back(buffered_fragment_);
        } else {
          ++count_corrupted_fragments_;
        }
      }
    }
    const std::lock_guard<std::mutex> lock(mutex_);
    have_fragment_for_tx_ = false;
    buffered_fragment_.clear();
    last_switch_to_tx_mode_ms_ = 0;
  }

  void drop_unread_received_fragments() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    if (!rx_fragments_.empty()) {
      const uint64_t current_time_ms{clock_->current_time_millis()};
      auto current_fragment = rx_fragments_.begin();
      size_t received_fragments{0};
      for (auto current = rx_fragments_.begin();
           current->first <= current_time_ms && current != rx_fragments_.end(); ++current) {
        current_fragment = current;
        ++received_fragments;
      }

      if (received_fragments > 0) {
        count_dropped_fragments_ += received_fragments - 1;
        rx_fragments_.erase(rx_fragments_.begin(), current_fragment);
      }
    }
  }

  void drop_unreceived_fragments() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    if (!rx_fragments_.empty()) {
      const uint64_t current_time_ms{clock_->current_time_millis()};
      auto current_fragment = rx_fragments_.begin();
      size_t received_fragments{0};
      for (auto current = rx_fragments_.begin();
           current->first <= current_time_ms && current != rx_fragments_.end(); ++current) {
        current_fragment = current;
        ++received_fragments;
      }

      if (received_fragments > 0) {
        count_dropped_fragments_ += received_fragments;
        rx_fragments_.erase(rx_fragments_.begin(), ++current_fragment);
      }
    }
  }

 public:
  MockRadioT() = default;
  MockRadioT(tvsc::hal::time::Clock& clock) : clock_(&clock) {}

  /**
   * Add a fragment to be received. This method helps configure the mock radio state.
   */
  void add_rx_fragment(uint64_t rx_timestamp, const Fragment<MTU>& fragment) {
    const std::lock_guard<std::mutex> lock(mutex_);
    rx_fragments_.insert({rx_timestamp, fragment});
  }

  size_t remaining_rx_fragment_count() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return rx_fragments_.size();
  }

  /**
   * Get collection of fragments that were transmitted successfully.
   */
  const std::vector<Fragment<MTU>>& sent_fragments() const {
    handle_fragment_transmit();
    const std::lock_guard<std::mutex> lock(mutex_);
    return sent_fragments_;
  }

  size_t count_dropped_fragments() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return count_dropped_fragments_;
  }

  size_t count_corrupted_fragments() const {
    const std::lock_guard<std::mutex> lock(mutex_);
    return count_corrupted_fragments_;
  }

  bool in_standby_mode() const override {
    const std::lock_guard<std::mutex> lock(mutex_);
    return current_mode_ == Mode::STANDBY;
  }

  bool in_rx_mode() const override {
    const std::lock_guard<std::mutex> lock(mutex_);
    return current_mode_ == Mode::RX;
  }

  bool in_tx_mode() const override {
    const std::lock_guard<std::mutex> lock(mutex_);
    return current_mode_ == Mode::TX;
  }

  /**
   * Reset the radio back to a default state and in standby mode. This may include a hardware
   * reset, such as powering down the module and powering it back on again, toggling an enable or
   * reset line, etc.
   *
   * The constructor for the radio should put it in the same default state. This is easily
   * accomplished by calling reset() at the end of the constructor.
   *
   * This call should be idempotent.
   */
  void reset() override {
    {
      const std::lock_guard<std::mutex> lock(mutex_);
      buffered_fragment_.clear();
    }
    set_standby_mode();
  }

  /**
   * Read the RSSI (Received Signal Strength Indicator) in dBm. Note that making this reading may
   * interrupt any ongoing rx or tx operation and will likely involve changing register values on
   * the radio.
   */
  float read_rssi_dbm() override {
    // Return a benign value for now. Consider adding logic to set different RSSI levels for mocking
    // different scenarios.
    return -85.f;
  }

  /**
   * Put the radio in a standby mode. Standby means that it is not receiving or transmitting.
   * This may be used to save power.
   *
   * This call should be idempotent.
   */
  void set_standby_mode() override {
    if (!in_standby_mode()) {
      handle_fragment_transmit();
      const std::lock_guard<std::mutex> lock(mutex_);
      current_mode_ = Mode::STANDBY;
    }
  }

  /**
   * Configure the radio to receive data.
   *
   * This call should be idempotent.
   */
  void set_receive_mode() override {
    if (!in_rx_mode()) {
      handle_fragment_transmit();
      drop_unreceived_fragments();
      const std::lock_guard<std::mutex> lock(mutex_);
      current_mode_ = Mode::RX;
    }
  }

  /**
   * Flag to poll if the radio has rx data available to read.
   */
  bool has_fragment_available() const override {
    if (in_rx_mode()) {
      drop_unread_received_fragments();
    } else {
      drop_unreceived_fragments();
    }
    const std::lock_guard<std::mutex> lock(mutex_);
    if (!rx_fragments_.empty()) {
      const uint64_t current_time_ms{clock_->current_time_millis()};
      const auto& begin{rx_fragments_.begin()};
      return begin->first <= current_time_ms;
    }
    return false;
  }

  /**
   * Read a fragment that has already been received by the radio. After being read, the
   * radio will discard the fragment.
   */
  void read_received_fragment(Fragment<MTU>& fragment) override {
    if (in_rx_mode()) {
      drop_unread_received_fragments();
    } else {
      drop_unreceived_fragments();
    }

    const std::lock_guard<std::mutex> lock(mutex_);
    if (!rx_fragments_.empty()) {
      const uint64_t current_time_ms{clock_->current_time_millis()};
      auto chosen_entry = rx_fragments_.begin();
      size_t fragments_received{0};
      for (auto current = rx_fragments_.begin();
           current->first <= current_time_ms && current != rx_fragments_.end(); ++current) {
        chosen_entry = current;
        ++fragments_received;
      }

      if (fragments_received > 0) {
        fragment = chosen_entry->second;
        count_dropped_fragments_ += fragments_received - 1;
        rx_fragments_.erase(rx_fragments_.begin(), ++chosen_entry);
        return;
      }
    }
    fragment.clear();
  }

  /**
   * Flag to poll if the radio is detecting channel activity. Before transmitting, the
   * radio should wait until this activity has cleared.
   *
   * Note that this method cannot be marked const, since it likely involves reading the RSSI level
   * which will involve changing register values, disrupting any ongoing RX, etc.
   */
  bool channel_activity_detected() override {
    // Consider changing this to an RSSI threshold and making the RSSI level configurable for
    // testing.
    return false;
  }

  bool is_transmitting_fragment() override {
    std::unique_lock<std::mutex> lock(mutex_);
    return have_fragment_for_tx_;
  }

  /**
   * Transmit a fragment.
   *
   * Returns true if the transmission was initiated, false if it could not start within the timeout.
   * True does not guarantee that it was received.
   */
  bool transmit_fragment(const Fragment<MTU>& fragment, uint16_t timeout_ms) override {
    const uint64_t abandon_time_ms{clock_->current_time_millis() + timeout_ms};

    // Wait until we have a clear channel to transmit.
    while (channel_activity_detected() && clock_->current_time_millis() < abandon_time_ms) {
      clock_->sleep_us(100);
    }
    if (clock_->current_time_millis() >= abandon_time_ms) {
      return false;
    }

    set_standby_mode();

    const std::lock_guard<std::mutex> lock(mutex_);
    buffered_fragment_ = fragment;
    have_fragment_for_tx_ = true;
    current_mode_ = Mode::TX;
    last_switch_to_tx_mode_ms_ = clock_->current_time_millis();
    return true;
  }
};

/**
 * Radio that has the same MTU as cheap hobbyist radios.
 */
using SmallBufferMockRadio = MockRadioT<65>;

using MockRadio = MockRadioT<256>;

/**
 * Radio that has the same MTU as most ethernet devices.
 */
using LargeBufferMockRadio = MockRadioT<1500>;

/**
 * Radio that has an MTU configured for jumbo ethernet frames.
 */
using JumboBufferMockRadio = MockRadioT<9000>;

}  // namespace tvsc::radio
