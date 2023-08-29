#pragma once

#include <cstdint>

#include "hal/time/time.h"
#include "radio/fragment.h"
#include "random/random.h"

namespace tvsc::radio {

/**
 * API managing the transactional states of a half-duplex radio.
 *
 * Configuration of the radio, frequency to use, modulation, etc. is handled by the
 * RadioConfiguration class and is outside the scope of this API. The RadioConfiguration is based on
 * protocol buffers with templated functions for each radio module. It is expected that the radio
 * modules themselves will have direct getters and setters and the configuration functions will be
 * specialized to call those getters and setters.
 */
template <size_t MTU>
class HalfDuplexRadio {
 public:
  static constexpr size_t MAX_MTU_VALUE{MTU};
  static constexpr size_t max_mtu() { return MTU; }

  virtual ~HalfDuplexRadio() = default;

  /**
   * The current MTU of the radio. By default, this is the compile-time constant MTU. But, it
   * is possible to make this value configurable to allow different MTUs in different scenarios.
   */
  virtual size_t mtu() const { return max_mtu(); }

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
  virtual void reset() = 0;

  /**
   * Read the RSSI (Received Signal Strength Indicator) in dBm. Note that making this reading may
   * interrupt any ongoing rx or tx operation and will likely involve changing register values on
   * the radio.
   */
  virtual float read_rssi_dbm() = 0;

  /**
   * How long, in milliseconds, it takes to measure the RSSI using this radio.
   */
  virtual uint16_t rssi_measurement_time_ms() = 0;

  /**
   * Put the radio in a standby mode. Standby means that it is not receiving or transmitting.
   * This may be used to save power. It may also be used during the setup of a transmission to
   * guarantee that the radio doesn't start receiving a fragment while it is preparing to
   * transmit.
   *
   * This call should be idempotent.
   */
  virtual void set_standby_mode() = 0;

  /**
   * Configure the radio to receive data.
   *
   * This call should be idempotent.
   */
  virtual void set_receive_mode() = 0;

  /**
   * Getters for the current mode.
   */
  virtual bool in_standby_mode() const = 0;
  virtual bool in_rx_mode() const = 0;

  /**
   * Note that there is no setter for TX mode. This mode is entered by initiating a transmission via
   * transmit_fragment().
   */
  virtual bool in_tx_mode() const = 0;

  /**
   * Flag to poll if the radio has rx data available to read.
   */
  virtual bool has_fragment_available() const = 0;

  /**
   * Read a fragment that has already been received by the radio. After being read, the
   * radio will discard the fragment.
   */
  virtual void read_received_fragment(Fragment<MTU>& fragment) = 0;

  /**
   * Flag to poll if the radio is detecting channel activity. Before transmitting, the
   * radio should wait until this activity has cleared.
   *
   * Note that this method cannot be marked const, since it likely involves reading the RSSI level
   * which will involve changing register values, disrupting any ongoing RX, etc.
   */
  virtual bool channel_activity_detected() = 0;

  /**
   * Transmit a fragment.
   *
   * Returns true if the transmission was initiated, false if it could not start within the timeout.
   * True does not guarantee that it was received.
   */
  virtual bool transmit_fragment(const Fragment<MTU>& fragment, uint16_t timeout_ms) = 0;

  /**
   * Returns true if this radio is currently transmitting; false, otherwise.
   */
  virtual bool is_transmitting_fragment() {
    // By default, being in TX mode means that the radio is transmitting a fragment.
    return in_tx_mode();
  }
};

}  // namespace tvsc::radio
