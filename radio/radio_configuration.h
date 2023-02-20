#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "radio/radio.pb.h"
#include "random/random.h"

namespace tvsc::radio {

/**
 * Generate a Value that is a range of float values from a to b. The value of the inclusivity
 * parameter dictates which of the endpoints is included.
 */
Value float_range(float a, float b,
                  RangeInclusivity inclusivity = RangeInclusivity::INCLUSIVE_INCLUSIVE) {
  Value result{};
  auto* range = result.add_ranged();
  auto* float_range = range->mutable_float_range();
  float_range->set_min(a);
  float_range->set_max(b);
  float_range->set_inclusive(inclusivity);
  return result;
}

/**
 * Generate a Value that is a range of 32-bit integer values from a to b. The value of the
 * inclusivity parameter dictates which of the endpoints is included.
 */
Value int32_range(int32_t a, int32_t b,
                  RangeInclusivity inclusivity = RangeInclusivity::INCLUSIVE_INCLUSIVE) {
  Value result{};
  auto* range = result.add_ranged();
  auto* int_range = range->mutable_int32_range();
  int_range->set_min(a);
  int_range->set_max(b);
  int_range->set_inclusive(inclusivity);
  return result;
}

/**
 * Generate a Value that is a range of 64-bit integer values from a to b. The value of the
 * inclusivity parameter dictates which of the endpoints is included.
 */
Value int64_range(int64_t a, int64_t b,
                  RangeInclusivity inclusivity = RangeInclusivity::INCLUSIVE_INCLUSIVE) {
  Value result{};
  auto* range = result.add_ranged();
  auto* int_range = range->mutable_int64_range();
  int_range->set_min(a);
  int_range->set_max(b);
  int_range->set_inclusive(inclusivity);
  return result;
}

/**
 * Generate a Value that is the set of discrete enumerated values.
 */
template <typename T>
Value enumerated(std::initializer_list<T> values) {
  Value result{};
  return result;
}

template <typename T>
T as(const DiscreteValue& value);

template <>
uint8_t as<uint8_t>(const DiscreteValue& value) {
  return static_cast<uint8_t>(value.int32_value());
}

template <>
uint16_t as<uint16_t>(const DiscreteValue& value) {
  return static_cast<uint16_t>(value.int32_value());
}

template <>
uint32_t as<uint32_t>(const DiscreteValue& value) {
  return static_cast<uint32_t>(value.int32_value());
}

template <>
uint64_t as<uint64_t>(const DiscreteValue& value) {
  return static_cast<uint64_t>(value.int64_value());
}

template <>
float as<float>(const DiscreteValue& value) {
  return value.float_value();
}

template <typename DriverT>
std::unordered_map<Function, Value> generate_capabilities_map();

template <typename DriverT>
DiscreteValue read_setting(DriverT& driver, Function function);

template <typename DriverT>
void write_setting(DriverT& driver, Function function, const DiscreteValue& value);

/**
 * This template defines an interface for configuring a radio. The specific radio type will be
 * driven by the template parameter.
 *
 * Note that we are trying to avoid an inheritance hierarchy here. We expect there to be many
 * different types of radios, even in the short term. We want a uniform way to interact with them
 * all, but we don't want the readability headaches that go with elaborate inheritance schemes.  Our
 * approach is to define a template and provide specializations of this template for each radio
 * type. These template specializations will be in their own header files and be locked away as
 * separate C++ libraries under Bazel. This scheme will also avoid any runtime overhead; the
 * compiler will only include the code that we need in each executable, and there will be no virtual
 * table indirections.
 */
template <typename DriverT>
class RadioConfiguration final {
 private:
  DriverT* const driver_;
  const std::unordered_map<Function, Value> capabilities_;
  const uint64_t expanded_id_;
  uint16_t id_;
  const std::string name_;

  std::unordered_map<Function, DiscreteValue> pending_settings_changes_{};

 public:
  RadioConfiguration(DriverT& driver, std::string_view name = "HopeRF RFM69HCW")
      : driver_(&driver),
        capabilities_(generate_capabilities_map<DriverT>()),
        expanded_id_(tvsc::random::generate_random_value<uint64_t>()),
        id_(expanded_id_ & static_cast<uint16_t>(0xffff)),
        name_(name) {
    // We know the number of settings at compile time, and we know that number will not change.
    // Because of this, we can reserve the exact number of buckets we need and use a load factor of
    // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
    // O(n) with this setup, we will still see good performance.
    pending_settings_changes_.max_load_factor(1.f);
    pending_settings_changes_.reserve(capabilities_.size());
  }

  uint64_t expanded_id() const { return expanded_id_; }

  uint32_t id() const { return id_; }
  void set_id(uint32_t id) { id_ = id; }

  const std::string& name() const { return name_; }

  std::vector<Function> get_configurable_functions() const {
    std::vector<Function> functions{};
    functions.reserve(capabilities_.size());
    for (const auto& [key, _] : capabilities_) {
      functions.push_back(key);
    }
    return functions;
  }

  Value get_valid_values(Function function) const { return capabilities_.at(function); }

  int32_t get_int32_value(Function function) const {
    const DiscreteValue value = read_setting<DriverT>(*driver_, function);
    return value.int32_value();
  }

  int64_t get_int64_value(Function function) const {
    const DiscreteValue value = read_setting<DriverT>(*driver_, function);
    return value.int64_value();
  }

  float get_float_value(Function function) const {
    const DiscreteValue value = read_setting<DriverT>(*driver_, function);
    return value.float_value();
  }

  void set_int32_value(Function function, int32_t value) {
    DiscreteValue discrete{};
    discrete.set_int32_value(value);
    pending_settings_changes_.insert({function, discrete});
  }

  void set_int64_value(Function function, int64_t value) {
    DiscreteValue discrete{};
    discrete.set_int64_value(value);
    pending_settings_changes_.insert({function, discrete});
  }

  void set_float_value(Function function, float value) {
    DiscreteValue discrete{};
    discrete.set_float_value(value);
    pending_settings_changes_.insert({function, discrete});
  }

  void commit_settings_changes() {
    // Write these settings to the driver. Assumes that every setting can be set independently.
    for (const auto& entry : pending_settings_changes_) {
      write_setting<DriverT>(*driver_, entry.first, entry.second);
    }
    pending_settings_changes_.clear();
  }

  void abort_settings_changes() { pending_settings_changes_.clear(); }
};

}  // namespace tvsc::radio
