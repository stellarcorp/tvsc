#pragma once

#include <ratio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "base/units.h"
#include "radio/radio.pb.h"
#include "radio/radio_configuration.h"
#include "radio/rf69hcw.h"

namespace tvsc::radio {

/**
 * These templates define an interface for configuring the HopeRF RFM69HCW radio module using the
 * RadioHead RH_RF69 Driver.
 *
 * Manufacturer's documentation for the radio module:
 * https://www.hoperf.com/modules/rf_transceiver/RFM69HCW.html
 *
 * Note: this class covers the high-power variant tuned for ~433MHz.
 * TODO(james): When implementing the next variant, add template parameters for the tuning and the
 * power-level.
 *
 * From the RadioHead library documentation on this module:
 *   Packet Format
 *
 *   All messages sent and received by this RH_RF69 Driver conform to this packet format:
 *
 *   - 4 octets PREAMBLE
 *   - 2 octets SYNC 0x2d, 0xd4 (configurable, so you can use this as a network filter)
 *   - 1 octet RH_RF69 payload length
 *   - 4 octets HEADER: (TO, FROM, ID, FLAGS)
 *   - 0 to 60 octets DATA
 *   - 2 octets CRC computed with CRC16(IBM), computed on HEADER and DATA
 *
 * But that description leaves off these details:
 *
 * - The PREAMBLE can be configured to be from 0-65535 octets, though the comments indicate that at
 * least 2 octets are needed for stable communications. There are two 8-bit registers to store the
 * length of the preamble.
 *
 * - The length, as well as the contents, of the SYNC words are configurable. Allowed values for the
 * length are actually [0,8] where zero implies that the SYNC functionality should be turned off.
 * Not sure why, but the RadioHead implementation restricts the sync word size to 4 octets.
 *
 * - The implementation does not allow for an empty payload, so the effective range for the DATA
 * segment is 1-60 octets.
 *
 * - Longer payloads are possible if encryption is off. There might be some regulatory implications
 * though. Without encryption, we cannot send flight or control messages per FCC regulations. Maybe
 * our Tx/Rx API just includes a boolean indicating if the message is a control message and manages
 * that aspect of the configuration directly for those messages?
 */

template <>
std::unordered_map<tvsc_radio_Function, tvsc_radio_Value> generate_capabilities_map<RF69HCW>() {
  std::unordered_map<tvsc_radio_Function, tvsc_radio_Value> capabilities{};
  // We know the number of settings at compile time, and we know that number will not change.
  // Because of this, we can reserve the exact number of buckets we need and use a load factor of
  // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
  // O(n) with this setup, we will still see good performance.
  capabilities.max_load_factor(1.f);

  // Adafruit says we can go from 400MHz to 460MHz. Manufacturer says 424MHz to 510MHz. We measured
  // the range as 405MHz to 510MHz.
  capabilities.insert(
      {tvsc_radio_Function_CARRIER_FREQUENCY_HZ, float_range(405'000'000, 510'000'000)});

  capabilities.insert(
      {tvsc_radio_Function_MODULATION_SCHEME,
       enumerated({tvsc_radio_ModulationTechnique_OOK, tvsc_radio_ModulationTechnique_FSK,
                   tvsc_radio_ModulationTechnique_MSK, tvsc_radio_ModulationTechnique_GFSK,
                   tvsc_radio_ModulationTechnique_GMSK})});

  capabilities.insert({tvsc_radio_Function_TX_POWER_DBM, int32_range(-2, 20)});

  // Note that it might be possible to go up to 65 bytes if we take
  // control of the header generation. The FIFOs have a size of 66 bytes,
  // but we would need to leave one byte for the message length.
  capabilities.insert({tvsc_radio_Function_MTU, int32_range(1, 60)});

  capabilities.insert({tvsc_radio_Function_PREAMBLE_LENGTH, int32_range(0, 0xffff)});

  capabilities.insert({tvsc_radio_Function_SYNC_WORDS_LENGTH, int32_range(0, 4)});

  capabilities.insert(
      {tvsc_radio_Function_LINE_CODING,
       enumerated({tvsc_radio_LineCoding_NONE, tvsc_radio_LineCoding_MANCHESTER_ORIGINAL,
                   tvsc_radio_LineCoding_WHITENING})});

  capabilities.insert(
      {tvsc_radio_Function_ENCRYPTION,
       enumerated({tvsc_radio_Encryption_NO_ENCRYPTION, tvsc_radio_Encryption_AES_128})});

  capabilities.insert({tvsc_radio_Function_BIT_RATE,
                       float_range(RF69HCW::RF69HCW_FXOSC / 0xffff, RF69HCW::RF69HCW_FXOSC)});

  capabilities.insert({tvsc_radio_Function_FREQUENCY_DEVIATION,
                       float_range(600.f, 0x3fff * RF69HCW::RF69HCW_FSTEP)});

  capabilities.insert(
      {tvsc_radio_Function_CHANNEL_ACTIVITY_DETECTION_TIMEOUT_MS, int32_range(0, 10000)});

  capabilities.insert(
      {tvsc_radio_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM, float_range(-127.5f, 0.f)});

  capabilities.insert(
      {tvsc_radio_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM, float_range(-127.5f, 0.f)});

  return capabilities;
}

inline void set_frequency_hz(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  float frequency = as<float>(value);
  driver.set_frequency_hz(frequency);
}

inline void get_frequency_hz(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 2;
  value.value.float_value = driver.get_frequency_hz();
}

inline void set_preamble_length(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  uint16_t length = as<uint16_t>(value);
  driver.set_preamble_length(length);
}

inline void get_preamble_length(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 0;
  value.value.int32_value = driver.get_preamble_length();
}

inline void set_sync_words_length(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  uint8_t length = as<uint8_t>(value);
  driver.set_sync_words_length(length);
}

inline void get_sync_words_length(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 0;
  value.value.int32_value = driver.get_sync_words_length();
}

inline void set_power_dbm(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  int8_t power = as<int8_t>(value);
  driver.set_power_dbm(power);
}

inline void get_power_dbm(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 0;
  value.value.int32_value = driver.get_power_dbm();
}

inline void set_modulation_scheme(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  tvsc_radio_ModulationTechnique modulation{as<tvsc_radio_ModulationTechnique>(value)};
  driver.set_modulation_scheme(modulation);
}

inline void get_modulation_scheme(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 0;
  value.value.int32_value = driver.get_modulation_scheme();
}

inline void set_line_coding(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  tvsc_radio_LineCoding coding{as<tvsc_radio_LineCoding>(value)};
  driver.set_line_coding(coding);
}

inline void get_line_coding(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 0;
  value.value.int32_value = driver.get_line_coding();
}

inline void set_bit_rate(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  float bit_rate{as<float>(value)};
  driver.set_bit_rate(bit_rate);
}

inline void get_bit_rate(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 2;
  value.value.float_value = driver.get_bit_rate();
}

inline void set_frequency_deviation_hz(RF69HCW& driver, const tvsc_radio_DiscreteValue& value) {
  float deviation{as<float>(value)};
  driver.set_frequency_deviation_hz(deviation);
}

inline void get_frequency_deviation_hz(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 2;
  value.value.float_value = driver.get_frequency_deviation_hz();
}

inline void set_channel_activity_detection_timeout_ms(RF69HCW& driver,
                                                      const tvsc_radio_DiscreteValue& value) {
  uint16_t timeout_ms{as<uint16_t>(value)};
  driver.set_channel_activity_detection_timeout_ms(timeout_ms);
}

inline void get_channel_activity_detection_timeout_ms(RF69HCW& driver,
                                                      tvsc_radio_DiscreteValue& value) {
  value.which_value = 0;
  value.value.int32_value = driver.get_channel_activity_detection_timeout_ms();
}

inline void set_receive_sensitivity_threshold_dbm(RF69HCW& driver,
                                                  const tvsc_radio_DiscreteValue& value) {
  float threshold_dbm{as<float>(value)};
  driver.set_receive_sensitivity_threshold_dbm(threshold_dbm);
}

inline void get_receive_sensitivity_threshold_dbm(RF69HCW& driver,
                                                  tvsc_radio_DiscreteValue& value) {
  value.which_value = 1;
  value.value.float_value = driver.get_receive_sensitivity_threshold_dbm();
}

inline void set_channel_activity_threshold_dbm(RF69HCW& driver,
                                               const tvsc_radio_DiscreteValue& value) {
  float threshold_dbm{as<float>(value)};
  driver.set_channel_activity_threshold_dbm(threshold_dbm);
}

inline void get_channel_activity_threshold_dbm(RF69HCW& driver, tvsc_radio_DiscreteValue& value) {
  value.which_value = 2;
  value.value.float_value = driver.get_channel_activity_threshold_dbm();
}

template <>
tvsc_radio_DiscreteValue read_setting<RF69HCW>(RF69HCW& driver, tvsc_radio_Function function) {
  tvsc_radio_DiscreteValue value{};
  switch (function) {
    case tvsc_radio_Function_CARRIER_FREQUENCY_HZ: {
      get_frequency_hz(driver, value);
      break;
    }
    case tvsc_radio_Function_PREAMBLE_LENGTH: {
      get_preamble_length(driver, value);
      break;
    }
    case tvsc_radio_Function_SYNC_WORDS_LENGTH: {
      get_sync_words_length(driver, value);
      break;
    }
    case tvsc_radio_Function_TX_POWER_DBM: {
      get_power_dbm(driver, value);
      break;
    }
    case tvsc_radio_Function_MODULATION_SCHEME: {
      get_modulation_scheme(driver, value);
      break;
    }
    case tvsc_radio_Function_LINE_CODING: {
      get_line_coding(driver, value);
      break;
    }
    case tvsc_radio_Function_BIT_RATE: {
      get_bit_rate(driver, value);
      break;
    }
    case tvsc_radio_Function_FREQUENCY_DEVIATION: {
      get_frequency_deviation_hz(driver, value);
      break;
    }
    case tvsc_radio_Function_CHANNEL_ACTIVITY_DETECTION_TIMEOUT_MS: {
      get_channel_activity_detection_timeout_ms(driver, value);
      break;
    }
    case tvsc_radio_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM: {
      get_receive_sensitivity_threshold_dbm(driver, value);
      break;
    }
    case tvsc_radio_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM: {
      get_channel_activity_threshold_dbm(driver, value);
      break;
    }
  }
  return value;
}

template <>
void write_setting<RF69HCW>(RF69HCW& driver, tvsc_radio_Function function,
                            const tvsc_radio_DiscreteValue& value) {
  switch (function) {
    case tvsc_radio_Function_CARRIER_FREQUENCY_HZ: {
      set_frequency_hz(driver, value);
      break;
    }
    case tvsc_radio_Function_PREAMBLE_LENGTH: {
      set_preamble_length(driver, value);
      break;
    }
    case tvsc_radio_Function_SYNC_WORDS_LENGTH: {
      set_sync_words_length(driver, value);
      break;
    }
    case tvsc_radio_Function_TX_POWER_DBM: {
      set_power_dbm(driver, value);
      break;
    }
    case tvsc_radio_Function_MODULATION_SCHEME: {
      set_modulation_scheme(driver, value);
      break;
    }
    case tvsc_radio_Function_LINE_CODING: {
      set_line_coding(driver, value);
      break;
    }
    case tvsc_radio_Function_BIT_RATE: {
      set_bit_rate(driver, value);
      break;
    }
    case tvsc_radio_Function_FREQUENCY_DEVIATION: {
      set_frequency_deviation_hz(driver, value);
      break;
    }
    case tvsc_radio_Function_CHANNEL_ACTIVITY_DETECTION_TIMEOUT_MS: {
      set_channel_activity_detection_timeout_ms(driver, value);
      break;
    }
    case tvsc_radio_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM: {
      set_receive_sensitivity_threshold_dbm(driver, value);
      break;
    }
    case tvsc_radio_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM: {
      set_channel_activity_threshold_dbm(driver, value);
      break;
    }
  }
}

}  // namespace tvsc::radio
