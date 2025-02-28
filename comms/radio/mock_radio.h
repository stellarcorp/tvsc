#pragma once

#include <limits>
#include <map>
#include <stdexcept>
#include <vector>

#include "comms/radio/half_duplex_radio.h"
#include "hal/output/output.h"
#include "hal/time/clock.h"
#include "hal/time/mock_clock.h"

namespace tvsc::comms::radio {

template <size_t MTU>
class MockRadio final : public HalfDuplexRadio<MTU>, tvsc::hal::time::Clockable {
 public:
  using TimeType = tvsc::hal::time::TimeType;

 private:
  // Amount of time in microseconds required to transmit a fragment, including ramping up and down
  // the amplifiers, DSP operations and DAC timing. The radio must stay continuously in TX
  // mode during this time.
  static constexpr uint16_t FRAGMENT_TRANSMIT_TIME_US{700};

  // Amount of time in microseconds required to measure the current RSSI level.
  static constexpr uint16_t RSSI_MEASUREMENT_TIME_US{500};

  // Timestamp of the last time a fragment was received. This should just increment in
  // fragment_receive_interval_us increments and is used to figure out when the next fragment should
  // be received.
  TimeType last_receive_time_us_{};

  // Timestamp of the last time the radio switched into TX mode. In order to transmit a
  // fragment, we must be in TX mode for at least FRAGMENT_TRANSMIT_TIME_US, else that fragment is
  // marked as corrupted.
  mutable TimeType last_switch_to_tx_mode_us_{0};

  // Collection of fragments that will be "received" when read_received_fragment() is called. These
  // fragments are indexed by a timestamp of when the fragment should be received. This timestamp is
  // in microseconds according to the Clock::current_time_micros() method, or the MockClock override
  // of the same method.
  mutable std::map<TimeType, Fragment<MTU>> rx_fragments_{};

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
  // FRAGMENT_TRANSMIT_TIME_US after the previous call to transmit_fragment(), the previous fragment
  // is corrupted. Basically, half-duplex radios need time to finish their transmissions before they
  // can reconfigured for other tasks.
  mutable size_t count_corrupted_fragments_{0};

  // Most radios we are working with only have storage for a single fragment. This may be the
  // fragment that was most recently received, or the fragment being transmitted. That means that
  // switching modes can drop/corrupt fragments.
  mutable Fragment<MTU> buffered_fragment_{};

  mutable bool have_fragment_for_tx_{false};

  /**
   * Drop any fragments that were received, but we weren't in a mode to receive them.
   */
  void process_reception(TimeType current_time_us) const {
    if (current_mode_ != Mode::RX) {
      if (!rx_fragments_.empty()) {
        auto current_fragment = rx_fragments_.begin();
        size_t received_fragments{0};
        for (auto current = rx_fragments_.begin();
             current->first < current_time_us && current != rx_fragments_.end(); ++current) {
          current_fragment = current;
          ++received_fragments;
        }

        if (received_fragments > 0) {
          count_dropped_fragments_ += received_fragments;
          rx_fragments_.erase(rx_fragments_.begin(), ++current_fragment);
        }
      }
    }
  }

  void process_ongoing_transmission(TimeType current_time_us) {
    if (have_fragment_for_tx_) {
      if (current_time_us - last_switch_to_tx_mode_us_ >= FRAGMENT_TRANSMIT_TIME_US &&  //
          current_mode_ == Mode::TX) {
        sent_fragments_.push_back(buffered_fragment_);

        have_fragment_for_tx_ = false;
        buffered_fragment_.clear();
        last_switch_to_tx_mode_us_ = 0;

        current_mode_ = Mode::STANDBY;
      } else if (current_mode_ != Mode::TX) {
        ++count_corrupted_fragments_;

        have_fragment_for_tx_ = false;
        buffered_fragment_.clear();
        last_switch_to_tx_mode_us_ = 0;
      }
    }
  }

  void update(TimeType current_time_us) noexcept override {
    process_reception(current_time_us);
    process_ongoing_transmission(current_time_us);
  }

  /**
   * Configure the radio to transmit data.
   *
   * This call should be idempotent.
   */
  void set_tx_mode() {
    current_mode_ = Mode::TX;

    const TimeType current_time_us{clock_->current_time_micros()};
    last_switch_to_tx_mode_us_ = current_time_us;
    update(current_time_us);
  }

 public:
  MockRadio(tvsc::hal::time::MockClock& clock) : Clockable(clock) {}

  /**
   * Add a fragment to be received. Use this method to configure the mock radio state.
   */
  void add_rx_fragment(tvsc::hal::time::TimeType rx_timestamp_us, const Fragment<MTU>& fragment) {
    rx_fragments_.insert({rx_timestamp_us, fragment});
  }

  size_t remaining_rx_fragment_count() const { return rx_fragments_.size(); }

  /**
   * Get collection of fragments that were transmitted successfully.
   */
  const std::vector<Fragment<MTU>>& sent_fragments() const { return sent_fragments_; }

  size_t count_dropped_fragments() const { return count_dropped_fragments_; }

  size_t count_corrupted_fragments() const { return count_corrupted_fragments_; }

  bool in_standby_mode() const override { return current_mode_ == Mode::STANDBY; }

  bool in_rx_mode() const override { return current_mode_ == Mode::RX; }

  bool in_tx_mode() const override { return current_mode_ == Mode::TX; }

  void reset() override {
    buffered_fragment_.clear();
    set_standby_mode();
  }

  float read_rssi_dbm() override {
    set_standby_mode();
    // Return a benign value for now. Consider adding logic to set different RSSI levels for mocking
    // different scenarios.
    return -85.f;
  }

  uint16_t rssi_measurement_time_us() const override { return RSSI_MEASUREMENT_TIME_US; }

  uint16_t fragment_transmit_time_us() const override { return FRAGMENT_TRANSMIT_TIME_US; }

  void set_standby_mode() override {
    current_mode_ = Mode::STANDBY;
    update(clock_->current_time_micros());
  }

  void set_receive_mode() override {
    current_mode_ = Mode::RX;
    update(clock_->current_time_micros());
  }

  bool has_fragment_available() const override {
    if (!rx_fragments_.empty()) {
      const TimeType current_time_us{clock_->current_time_micros()};
      const auto& begin{rx_fragments_.begin()};
      return begin->first <= current_time_us;
    }
    return false;
  }

  void read_received_fragment(Fragment<MTU>& fragment) override {
    if (!rx_fragments_.empty()) {
      const TimeType current_time_us{clock_->current_time_micros()};
      auto chosen_entry = rx_fragments_.begin();
      size_t fragments_received{0};
      for (auto current = rx_fragments_.begin();
           current->first <= current_time_us && current != rx_fragments_.end(); ++current) {
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

  bool channel_activity_detected() override {
    // Consider changing this to an RSSI threshold and making the RSSI level configurable for
    // testing.
    return false;
  }

  bool is_transmitting_fragment() override { return have_fragment_for_tx_; }

  bool transmit_fragment(const Fragment<MTU>& fragment) override {
    if (fragment.total_length() == 0) {
      tvsc::hal::output::println("fragment.length is zero.");
      return false;
    }

    set_standby_mode();

    buffered_fragment_ = fragment;
    have_fragment_for_tx_ = true;
    set_tx_mode();

    return true;
  }
};

/**
 * Radio that has the same MTU as cheap hobbyist radios.
 */
using SmallBufferMockRadio = MockRadio<65>;

using StandardMockRadio = MockRadio<256>;

/**
 * Radio that has the same MTU as most ethernet devices.
 */
using LargeBufferMockRadio = MockRadio<1500>;

/**
 * Radio that has an MTU configured for jumbo ethernet frames.
 */
using JumboBufferMockRadio = MockRadio<9000>;

}  // namespace tvsc::comms::radio
