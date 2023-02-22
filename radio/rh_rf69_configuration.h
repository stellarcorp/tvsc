#pragma once

#include <ratio>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "RadioHead/RH_RF69.h"
#include "base/units.h"
#include "radio/radio.pb.h"
#include "radio/radio_configuration.h"

namespace tvsc::radio {

constexpr uint8_t SYNC_WORDS[] = "SR90tvsc";

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
std::unordered_map<Function, Value> generate_capabilities_map<RH_RF69>() {
  std::unordered_map<Function, Value> capabilities{};
  // We know the number of settings at compile time, and we know that number will not change.
  // Because of this, we can reserve the exact number of buckets we need and use a load factor of
  // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
  // O(n) with this setup, we will still see good performance.
  capabilities.max_load_factor(1.f);

  // Adafruit says we can go from 400MHz to 460MHz. Manufacturer says 424MHz to 510MHz. We limit
  // ourselves to the intersection of these ranges, since we probably don't care to step too far
  // from 433MHz anyway.
  capabilities.insert({Function::CARRIER_FREQUENCY_HZ,
                       float_range(in_unit<std::mega>(424), in_unit<std::mega>(460))});

  capabilities.insert(
      {Function::MODULATION_SCHEME,
       enumerated({ModulationTechnique::OOK, ModulationTechnique::FSK, ModulationTechnique::MSK,
                   ModulationTechnique::GFSK, ModulationTechnique::GMSK})});

  capabilities.insert({Function::TX_POWER_DBM, int32_range(11, 17)});

  // Note that it might be possible to go up to 65 bytes if we take
  // control of the header generation. The FIFOs have a size of 66 bytes,
  // but we would need to leave one byte for the message length.
  capabilities.insert({Function::MTU, int32_range(1, 60)});

  capabilities.insert({Function::PREAMBLE_LENGTH, int32_range(0, 65535)});

  capabilities.insert({Function::SYNC_WORDS_LENGTH, int32_range(0, 4)});

  capabilities.insert(
      {Function::LINE_CODING,
       enumerated({LineCoding::NONE, LineCoding::MANCHESTER_ORIGINAL, LineCoding::WHITENING})});

  capabilities.insert(
      {Function::ENCRYPTION, enumerated({Encryption::NO_ENCRYPTION, Encryption::AES_128})});

  return capabilities;
}

template <>
DiscreteValue read_setting<RH_RF69>(RH_RF69& driver, Function function) {
  return as_discrete_value(0);
}

template <>
void write_setting<RH_RF69>(RH_RF69& driver, Function function, const DiscreteValue& value) {
  using std::to_string;
  switch (function) {
    case CARRIER_FREQUENCY_HZ: {
      float value_hz = as<float>(value);
      // RadioHead RH_RF69 code expects this value in MHz.
      driver.setFrequency(value_hz / 1'000'000);
      break;
    }
    case PREAMBLE_LENGTH: {
      driver.setPreambleLength(as<uint16_t>(value));
      break;
    }
    case SYNC_WORDS_LENGTH: {
      driver.setSyncWords(SYNC_WORDS, as<uint8_t>(value));
      break;
    }
  }
}

}  // namespace tvsc::radio
