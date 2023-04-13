#pragma once

#include "random/random.h"

namespace tvsc::radio {

/**
 * API managing the transactional states of a half-duplex transceiver.
 *
 * Configuration of the transceiver, frequency to use, modulation, etc. is handled by the
 * RadioConfiguration class and is outside the scope of this API. The RadioConfiguration is based on
 * protocol buffers with templated functions for each radio module. It is expected that the radio
 * modules themselves will have direct getters and setters and the configuration functions will be
 * specialized to call those getters and setters.
 */
template <size_t MTU>
class HalfDuplexTransceiver {
 public:
  static constexpr size_t max_mtu() { return MTU; }

  virtual ~HalfDuplexTransceiver() = default;

  /**
   * The current MTU of the transceiver. By default, this is the compile-time constant MTU. But, it
   * is possible to make this value configurable to allow different MTUs in different scenarios.
   */
  virtual size_t mtu() const { return max_mtu(); }

  /**
   * Reset the transceiver back to a default state and in standby mode. This may include a hardware
   * reset, such as powering down the module and powering it back on again, toggling an enable or
   * reset line, etc.
   *
   * The constructor for the transceiver should put it in the same default state. This is easily
   * accomplished by calling reset() at the end of the constructor.
   *
   * This call should be idempotent.
   */
  virtual void reset() = 0;

  /**
   * Read the RSSI (Received Signal Strength Indicator) in dBm. Note that making this reading may
   * interrupt any ongoing rx or tx operation and will likely involve changing register values on
   * the transceiver.
   */
  virtual float read_rssi_dbm() = 0;

  /**
   * Put the transceiver in a standby mode. Standby means that it is not receiving or transmitting.
   * This may be used to save power.
   *
   * This call should be idempotent.
   */
  virtual void standby() = 0;

  /**
   * Configure the transceiver to receive data.
   *
   * This call should be idempotent.
   */
  virtual void receive() = 0;

  /**
   * Flag to poll if the transceiver has rx data available to read.
   */
  virtual bool has_fragment_available() const = 0;

  /**
   * Block up to timeout_ms milliseconds for a fragment to be received.
   */
  virtual bool wait_fragment_available(uint16_t timeout_ms) const {
    if (has_fragment_available()) {
      return true;
    }

    static constexpr uint16_t poll_delay_ms{1};
    auto start = tvsc::hal::time::time_millis();
    while ((tvsc::hal::time::time_millis() - start) < timeout_ms) {
      if (has_fragment_available()) {
        return true;
      }
      tvsc::hal::time::delay_ms(poll_delay_ms);
    }
    return false;
  }

  /**
   * Read a fragment that has already been received by the transceiver. After being read, the
   * transceiver will discard the fragment.
   *
   * The length parameter should contain the capacity of the buffer when called. This capacity
   * should probably be the current mtu() or larger. Any remaining data in the fragment,
   * beyond the capacity of the buffer, will be dropped after this call. On return, length will
   * contain the number of bytes actually read.
   */
  virtual void read_received_fragment(uint8_t* buffer, uint8_t* length) = 0;

  /**
   * Flag to poll if the transceiver is detecting channel activity. Before transmitting, the
   * transceiver should wait until this activity has cleared.
   *
   * Note that this method cannot be marked const, since it likely involves reading the RSSI level
   * which will involve changing register values, disrupting any ongoing RX, etc.
   */
  virtual bool channel_activity_detected() = 0;

  /**
   * Block up to timeout_ms milliseconds for channel activity to clear.
   *
   * Note that this method cannot be marked const, since it likely involves reading the RSSI level
   * which will involve changing register values, disrupting any ongoing RX, etc.
   */
  virtual bool wait_channel_activity_clear(uint16_t timeout_ms) {
    // Wait for the detected channel activity to finish.
    // We use a random delay here to ensure that when we do try to transmit, we aren't immediately
    // colliding with another transmitter. See
    // https://en.wikipedia.org/wiki/Distributed_coordination_function for a more detailed
    // explanation.
    auto t = tvsc::hal::time::time_millis();
    while (channel_activity_detected()) {
      if (tvsc::hal::time::time_millis() - t > timeout_ms) {
        return false;
      }
      tvsc::hal::time::delay_ms(tvsc::random::generate_random_value<uint16_t>(
          static_cast<uint16_t>(1), std::min(static_cast<uint16_t>(25), timeout_ms)));
    }

    return true;
  }

  /**
   * Block up to timeout_ms milliseconds for a fragment to be transmitted.
   */
  virtual bool wait_fragment_transmitted(uint16_t timeout_ms) const = 0;

  /**
   * Transmit a fragment.
   *
   * Returns true if the transmission was initiated, false if it could not start within the timeout.
   * True does not guarantee that it was received.
   */
  virtual bool transmit_fragment(const uint8_t* buffer, uint8_t length, uint16_t timeout_ms) = 0;

  /**
   * Helper function to receive a fragment.
   */
  bool receive_fragment(uint8_t* buffer, uint8_t* length, uint16_t timeout_ms) {
    // Put the transceiver into receive mode.
    receive();
    // Wait until data is avaiable, up to timeout_ms milliseconds.
    if (wait_fragment_available(timeout_ms)) {
      // A fragment is available. Read it into the provided buffer.
      read_received_fragment(buffer, length);
      return true;
    } else {
      // No fragment was available within timeout_ms.
      *length = 0;
      return false;
    }
  }
};

}  // namespace tvsc::radio
