#pragma once

#include <ratio>
#include <string>
#include <unordered_map>
#include <vector>

#include "RadioHead/RH_RF69.h"
#include "base/units.h"
#include "radio/radio.pb.h"
#include "radio/radio_configuration.h"
#include "random/random.h"

namespace tvsc::radio {

/**
 * This template defines an interface for configuring the HopeRF RFM69HCW radio module using the
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
 * least 2 octets are needed for stable communications. (TODO(james): Verify. There are two 8-bit
 * registers to store the length of the preamble, but all of those values may not be valid.)
 *
 * - The length, as well as the contents, of the SYNC words are configurable. Allowed values for the
 * length are [0,4].
 *
 * - The implementation does not allow for an empty payload, so the effective range for the DATA
 * segment is 1-60 octets.
 *
 * - Longer payloads are possible if encryption is off. There might be some regulatory implications
 * though. Without encryption, we can send flight or control messages per FCC regulations. Maybe our
 * Tx/Rx API just includes a boolean indicating if the message is a control message and manages that
 * aspect of the configuration directly for those messages?
 */
template <>
class RadioConfiguration<RH_RF69> final {
 private:
  RH_RF69* const driver_;
  uint16_t id_{};
  uint64_t expanded_id_{};

  std::unordered_map<Function, Value> capabilities_{};

  std::unordered_map<Function, DiscreteValue> settings_{};
  std::unordered_map<Function, DiscreteValue> pending_settings_changes_{};

  void generate_ids() {
    expanded_id_ = tvsc::random::generate_random_value<uint64_t>();
    id_ = expanded_id_ &
          static_cast<uint16_t>(0xffff);  // The id starts as just the 16 LSB of the expanded id.
  }

  void commit_setting(Function function, const DiscreteValue& value) {}

 public:
  RadioConfiguration(RH_RF69& driver) : driver_(&driver) {
    // We know the number of settings at compile time, and we know that number will not change.
    // Because of this, we can reserve the exact number of buckets we need and use a load factor of
    // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
    // O(n) with this setup, we will still see good performance.
    capabilities_.max_load_factor(1.f);
    settings_.max_load_factor(1.f);
    pending_settings_changes_.max_load_factor(1.f);

    // Adafruit says we can go from 400MHz to 460MHz. Manufacturer says 424MHz to 510MHz. We limit
    // ourselves to the intersection of these ranges, since we probably don't care to step too far
    // from 433MHz anyway.
    capabilities_.insert({Function::CARRIER_FREQUENCY_HZ,
                          float_range(in_unit<std::mega>(424), in_unit<std::mega>(460))});

    capabilities_.insert(
        {Function::MODULATION_SCHEME,
         enumerated({ModulationTechnique::OOK, ModulationTechnique::FSK, ModulationTechnique::MSK,
                     ModulationTechnique::GFSK, ModulationTechnique::GMSK})});

    capabilities_.insert({Function::TX_POWER_DBM, int32_range(11, 17)});

    // Note that it might be possible to go up to 65 bytes if we take
    // control of the header generation. The FIFOs have a size of 66 bytes,
    // but we would need to leave one byte for the message length.
    capabilities_.insert({Function::MTU, int32_range(1, 60)});

    capabilities_.insert({Function::PREAMBLE_LENGTH, int32_range(0, 65535)});

    capabilities_.insert({Function::SYNC_WORDS_LENGTH, int32_range(0, 4)});

    capabilities_.insert(
        {Function::LINE_CODING,
         enumerated({LineCoding::NONE, LineCoding::MANCHESTER_ORIGINAL, LineCoding::WHITENING})});

    capabilities_.insert(
        {Function::ENCRYPTION, enumerated({Encryption::NO_ENCRYPTION, Encryption::AES_128})});

    generate_ids();
  }

  uint64_t expanded_id() const { return 0L; }

  uint32_t id() const { return 0; }
  void set_id(uint32_t id) { id_ = id; }

  const std::string& name() const { return ""; }

  std::vector<Function> get_configurable_functions() const { return {}; }

  int32_t get_int32_value(Function function) const { return 0; }
  int64_t get_int64_value(Function function) const { return 0L; }
  float get_float_value(Function function) const { return 0.f; }

  void set_int32_value(Function function, int32_t value) const {}
  void set_int64_value(Function function, int64_t value) const {}
  void set_float_value(Function function, float value) const {}

  Value get_valid_values(Function function) const { return Value{}; }

  void commit_settings_changes() {
    // Write these settings to the driver.
    for (const auto& entry : pending_settings_changes_) {
      commit_setting(entry.first, entry.second);
    }

    // Move the pending settings into the settings map.
    for (auto& entry : pending_settings_changes_) {
      settings_.at(entry.first) = std::move(entry.second);
    }
    pending_settings_changes_.clear();
  }

  void abort_settings_changes() { pending_settings_changes_.clear(); }
};

}  // namespace tvsc::radio
