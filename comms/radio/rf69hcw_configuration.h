#pragma once

#include <ratio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "base/units.h"
#include "comms/radio/nanopb_proto/radio.pb.h"
#include "comms/radio/radio_configuration.h"
#include "comms/radio/rf69hcw.h"

namespace tvsc::comms::radio {

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
 * - The implementation does not allow for an empty payload. Also, the hardware FIFO is documented
 * as 66 bytes, but requires the first byte as a length. So the effective range for the DATA segment
 * is likely 1-65 octets.
 *
 * - Longer payloads are possible if encryption is off. There might be some regulatory implications
 * though. Without encryption, we cannot send flight or control messages per FCC regulations. Maybe
 * our Tx/Rx API just includes a boolean indicating if the message is a control message and manages
 * that aspect of the configuration directly for those messages?
 */

template <>
inline std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_Value>
generate_capabilities_map<RF69HCW>() {
  std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_Value> capabilities{};
  // We know the number of settings at compile time, and we know that number will not change.
  // Because of this, we can reserve the exact number of buckets we need and use a load factor of
  // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
  // O(n) with this setup, we will still see good performance.
  capabilities.max_load_factor(1.f);
  capabilities.reserve(16);

  // Adafruit says we can go from 400MHz to 460MHz. Manufacturer says 424MHz to 510MHz. We measured
  // the range as 405MHz to 510MHz.
  capabilities.insert(
      {tvsc_comms_radio_nano_Function_CARRIER_FREQUENCY_HZ, float_range(405'000'000, 510'000'000)});

  capabilities.insert(
      {tvsc_comms_radio_nano_Function_MODULATION_SCHEME,
       enumerated({tvsc_comms_radio_nano_ModulationTechnique_OOK, tvsc_comms_radio_nano_ModulationTechnique_FSK,
                   tvsc_comms_radio_nano_ModulationTechnique_MSK,
                   tvsc_comms_radio_nano_ModulationTechnique_GFSK,
                   tvsc_comms_radio_nano_ModulationTechnique_GMSK})});

  capabilities.insert({tvsc_comms_radio_nano_Function_TX_POWER_DBM, int32_range(-2, 20)});

  // Note that it might be possible to go up to 65 bytes if we take
  // control of the header generation. The FIFOs have a size of 66 bytes,
  // but we would need to leave one byte for the message length.
  capabilities.insert({tvsc_comms_radio_nano_Function_MTU, int32_range(1, 60)});

  capabilities.insert({tvsc_comms_radio_nano_Function_PREAMBLE_LENGTH, int32_range(0, 0xffff)});

  capabilities.insert({tvsc_comms_radio_nano_Function_SYNC_WORDS_LENGTH, int32_range(0, 8)});

  capabilities.insert(
      {tvsc_comms_radio_nano_Function_LINE_CODING,
       enumerated({tvsc_comms_radio_nano_LineCoding_NONE, tvsc_comms_radio_nano_LineCoding_MANCHESTER_ORIGINAL,
                   tvsc_comms_radio_nano_LineCoding_WHITENING})});

  capabilities.insert(
      {tvsc_comms_radio_nano_Function_ENCRYPTION,
       enumerated({tvsc_comms_radio_nano_Encryption_NO_ENCRYPTION, tvsc_comms_radio_nano_Encryption_AES_128})});

  capabilities.insert({tvsc_comms_radio_nano_Function_BIT_RATE,
                       float_range(RF69HCW::RF69HCW_FXOSC / 0xffff, RF69HCW::RF69HCW_FXOSC)});

  capabilities.insert({tvsc_comms_radio_nano_Function_FREQUENCY_DEVIATION,
                       float_range(600.f, 0x3fff * RF69HCW::RF69HCW_FSTEP)});

  capabilities.insert(
      {tvsc_comms_radio_nano_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM, float_range(-127.5f, 0.f)});

  capabilities.insert(
      {tvsc_comms_radio_nano_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM, float_range(-127.5f, 0.f)});

  return capabilities;
}

inline void set_frequency_hz(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  float frequency = as<float>(value);
  driver.set_frequency_hz(frequency);
}

inline void get_frequency_hz(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
  value.value.float_value = driver.get_frequency_hz();
}

inline void set_preamble_length(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  uint16_t length = as<uint16_t>(value);
  driver.set_preamble_length(length);
}

inline void get_preamble_length(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  value.value.int32_value = driver.get_preamble_length();
}

inline void set_sync_words_length(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  uint8_t length = as<uint8_t>(value);
  driver.set_sync_words_length(length);
}

inline void get_sync_words_length(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  value.value.int32_value = driver.get_sync_words_length();
}

inline void set_power_dbm(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  int8_t power = as<int8_t>(value);
  driver.set_power_dbm(power);
}

inline void get_power_dbm(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  value.value.int32_value = driver.get_power_dbm();
}

inline void set_modulation_scheme(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  tvsc_comms_radio_nano_ModulationTechnique modulation{as<tvsc_comms_radio_nano_ModulationTechnique>(value)};
  driver.set_modulation_scheme(modulation);
}

inline void get_modulation_scheme(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  value.value.int32_value = driver.get_modulation_scheme();
}

inline void set_line_coding(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  tvsc_comms_radio_nano_LineCoding coding{as<tvsc_comms_radio_nano_LineCoding>(value)};
  driver.set_line_coding(coding);
}

inline void get_line_coding(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_int32_value_tag;
  value.value.int32_value = driver.get_line_coding();
}

inline void set_bit_rate(RF69HCW& driver, const tvsc_comms_radio_nano_DiscreteValue& value) {
  float bit_rate{as<float>(value)};
  driver.set_bit_rate(bit_rate);
}

inline void get_bit_rate(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
  value.value.float_value = driver.get_bit_rate();
}

inline void set_frequency_deviation_hz(RF69HCW& driver,
                                       const tvsc_comms_radio_nano_DiscreteValue& value) {
  float deviation{as<float>(value)};
  driver.set_frequency_deviation_hz(deviation);
}

inline void get_frequency_deviation_hz(RF69HCW& driver, tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
  value.value.float_value = driver.get_frequency_deviation_hz();
}

inline void set_receive_sensitivity_threshold_dbm(RF69HCW& driver,
                                                  const tvsc_comms_radio_nano_DiscreteValue& value) {
  float threshold_dbm{as<float>(value)};
  driver.set_receive_sensitivity_threshold_dbm(threshold_dbm);
}

inline void get_receive_sensitivity_threshold_dbm(RF69HCW& driver,
                                                  tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
  value.value.float_value = driver.get_receive_sensitivity_threshold_dbm();
}

inline void set_channel_activity_threshold_dbm(RF69HCW& driver,
                                               const tvsc_comms_radio_nano_DiscreteValue& value) {
  float threshold_dbm{as<float>(value)};
  driver.set_channel_activity_threshold_dbm(threshold_dbm);
}

inline void get_channel_activity_threshold_dbm(RF69HCW& driver,
                                               tvsc_comms_radio_nano_DiscreteValue& value) {
  value.which_value = tvsc_comms_radio_nano_DiscreteValue_float_value_tag;
  value.value.float_value = driver.get_channel_activity_threshold_dbm();
}

template <>
inline tvsc_comms_radio_nano_DiscreteValue read_setting<RF69HCW>(RF69HCW& driver,
                                                           tvsc_comms_radio_nano_Function function) {
  tvsc_comms_radio_nano_DiscreteValue value{};
  switch (function) {
    case tvsc_comms_radio_nano_Function_CARRIER_FREQUENCY_HZ: {
      get_frequency_hz(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_PREAMBLE_LENGTH: {
      get_preamble_length(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_SYNC_WORDS_LENGTH: {
      get_sync_words_length(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_TX_POWER_DBM: {
      get_power_dbm(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_MODULATION_SCHEME: {
      get_modulation_scheme(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_LINE_CODING: {
      get_line_coding(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_BIT_RATE: {
      get_bit_rate(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_FREQUENCY_DEVIATION: {
      get_frequency_deviation_hz(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM: {
      get_receive_sensitivity_threshold_dbm(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM: {
      get_channel_activity_threshold_dbm(driver, value);
      break;
    }
  }
  return value;
}

template <>
inline void write_setting<RF69HCW>(RF69HCW& driver, tvsc_comms_radio_nano_Function function,
                                   const tvsc_comms_radio_nano_DiscreteValue& value) {
  switch (function) {
    case tvsc_comms_radio_nano_Function_CARRIER_FREQUENCY_HZ: {
      set_frequency_hz(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_PREAMBLE_LENGTH: {
      set_preamble_length(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_SYNC_WORDS_LENGTH: {
      set_sync_words_length(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_TX_POWER_DBM: {
      set_power_dbm(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_MODULATION_SCHEME: {
      set_modulation_scheme(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_LINE_CODING: {
      set_line_coding(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_BIT_RATE: {
      set_bit_rate(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_FREQUENCY_DEVIATION: {
      set_frequency_deviation_hz(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM: {
      set_receive_sensitivity_threshold_dbm(driver, value);
      break;
    }
    case tvsc_comms_radio_nano_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM: {
      set_channel_activity_threshold_dbm(driver, value);
      break;
    }
  }
}

inline std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_DiscreteValue>
high_throughput_configuration() {
  std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_DiscreteValue> configuration{};
  configuration.max_load_factor(1.f);
  configuration.reserve(16);

  configuration.insert(
      {tvsc_comms_radio_nano_Function_CARRIER_FREQUENCY_HZ, tvsc::comms::radio::as_discrete_value(433e6f)});

  configuration.insert(
      {tvsc_comms_radio_nano_Function_TX_POWER_DBM, tvsc::comms::radio::as_discrete_value<int8_t>(13)});

  // Successful values:
  // 0x0f
  // 0x10
  // 0x11
  // 0x14
  // 0x15
  // 0x01 <- Probably too small for high bit rates and high duty cycles.
  // Unsuccessful values:
  // 0xff
  // 0x7f
  // 0x3f
  // 0x1f
  // 0x1a
  // 0x18 -- intermittent
  // 0x17 -- intermittent
  // 0x16 -- intermittent
  // 0x00 -- intermittent
  // Seems to be used only during TX. The receiver watches for the preamble to stop, but ignores
  // this particular setting.
  configuration.insert(
      {tvsc_comms_radio_nano_Function_PREAMBLE_LENGTH, tvsc::comms::radio::as_discrete_value<uint16_t>(0x08)});

  // Successful values:
  // 8
  // 2
  // Unsuccessful values:
  // 0
  // 1 -- intermittent
  // The transmitter and receiver must agree both on length and content of the sync words.
  configuration.insert(
      {tvsc_comms_radio_nano_Function_SYNC_WORDS_LENGTH, tvsc::comms::radio::as_discrete_value<uint8_t>(2)});

  configuration.insert({tvsc_comms_radio_nano_Function_MODULATION_SCHEME,
                        tvsc::comms::radio::as_discrete_value(tvsc_comms_radio_nano_ModulationTechnique_GFSK)});

  // WHITENING seems to perform better at high bit rates & high duty cycles.
  // MANCHESTER_ORIGINAL performs well, but results in dropped packets with high bit rates and duty
  // cycles.
  configuration.insert({tvsc_comms_radio_nano_Function_LINE_CODING,
                        tvsc::comms::radio::as_discrete_value(tvsc_comms_radio_nano_LineCoding_WHITENING)});

  const float bit_rate{290000.f};
  const float freq_dev = std::min(500000.f - bit_rate / 2.f, 1.4f * bit_rate);

  configuration.insert(
      {tvsc_comms_radio_nano_Function_BIT_RATE, tvsc::comms::radio::as_discrete_value<float>(bit_rate)});

  // Seems to be used only during TX. The receiver detects this spread and adjusts to the sender's
  // value (likely some significant limits to this), but ignores this particular setting.
  configuration.insert(
      {tvsc_comms_radio_nano_Function_FREQUENCY_DEVIATION, as_discrete_value<float>(freq_dev)});

  configuration.insert({tvsc_comms_radio_nano_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM,
                        tvsc::comms::radio::as_discrete_value<float>(-50.f)});

  configuration.insert(
      {tvsc_comms_radio_nano_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM,
       // Initialize these thresholds to the same value.
       configuration.at(tvsc_comms_radio_nano_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM)});

  return configuration;
}

inline std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_DiscreteValue>
standard_configuration() {
  std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_DiscreteValue> configuration{};
  configuration.max_load_factor(1.f);
  configuration.reserve(16);

  configuration.insert(
      {tvsc_comms_radio_nano_Function_CARRIER_FREQUENCY_HZ, tvsc::comms::radio::as_discrete_value(433e6f)});

  configuration.insert(
      {tvsc_comms_radio_nano_Function_TX_POWER_DBM, tvsc::comms::radio::as_discrete_value<int8_t>(13)});

  // Successful values:
  // 0x0f
  // 0x10
  // 0x11
  // 0x14
  // 0x15
  // 0x01 <- Probably too small for high bit rates and high duty cycles.
  // Unsuccessful values:
  // 0xff
  // 0x7f
  // 0x3f
  // 0x1f
  // 0x1a
  // 0x18 -- intermittent
  // 0x17 -- intermittent
  // 0x16 -- intermittent
  // 0x00 -- intermittent
  // Seems to be used only during TX. The receiver watches for the preamble to stop, but ignores
  // this particular setting.
  configuration.insert(
      {tvsc_comms_radio_nano_Function_PREAMBLE_LENGTH, tvsc::comms::radio::as_discrete_value<uint16_t>(0x10)});

  // Successful values:
  // 8
  // 2
  // Unsuccessful values:
  // 0
  // 1 -- intermittent
  // The transmitter and receiver must agree both on length and content of the sync words.
  configuration.insert(
      {tvsc_comms_radio_nano_Function_SYNC_WORDS_LENGTH, tvsc::comms::radio::as_discrete_value<uint8_t>(8)});

  configuration.insert({tvsc_comms_radio_nano_Function_MODULATION_SCHEME,
                        tvsc::comms::radio::as_discrete_value(tvsc_comms_radio_nano_ModulationTechnique_FSK)});

  // WHITENING seems to perform better at high bit rates & high duty cycles.
  // MANCHESTER_ORIGINAL performs well, but results in dropped packets with high bit rates and duty
  // cycles.
  configuration.insert({tvsc_comms_radio_nano_Function_LINE_CODING,
                        tvsc::comms::radio::as_discrete_value(tvsc_comms_radio_nano_LineCoding_WHITENING)});

  const float bit_rate{12500.f};
  const float freq_dev = std::min(500000.f - bit_rate / 2.f, 1.4f * bit_rate);

  configuration.insert(
      {tvsc_comms_radio_nano_Function_BIT_RATE, tvsc::comms::radio::as_discrete_value<float>(bit_rate)});

  // Seems to be used only during TX. The receiver detects this spread and adjusts to the sender's
  // value (likely some significant limits to this), but ignores this particular setting.
  configuration.insert(
      {tvsc_comms_radio_nano_Function_FREQUENCY_DEVIATION, as_discrete_value<float>(freq_dev)});

  configuration.insert({tvsc_comms_radio_nano_Function_RECEIVE_SENSITIVITY_THRESHOLD_DBM,
                        tvsc::comms::radio::as_discrete_value<float>(-95.f)});

  // Note that the CAD threshold should be lower than the RX threshold, not higher. There are
  // signals that are being transmitted that are too weak for us to properly receive and decode.
  // These thresholds should be designed to avoid interfering with another transmitter, even if the
  // transmission is too weak for us to receive.
  // TODO(James): Determine why the RX and TX thresholds need to be inverted to get successful
  // behavior.
  // configuration.insert({tvsc_comms_radio_nano_Function_CHANNEL_ACTIVITY_THRESHOLD_DBM,
  //                       tvsc::comms::radio::as_discrete_value<float>(-0.5f)});

  return configuration;
}

template <>
inline std::unordered_map<tvsc_comms_radio_nano_Function, tvsc_comms_radio_nano_DiscreteValue>
default_configuration<RF69HCW>() {
  return standard_configuration();
}

}  // namespace tvsc::comms::radio