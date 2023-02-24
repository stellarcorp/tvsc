/**
 * This file contains the function templates that represent the API for configuring any radio we
 * work with. These should be specialized to create bindings for the driver of the radio module. It
 * is also possible to specialize RadioConfiguration itself, though it will be more difficult to
 * keep specializations in sync with the primary template below.
 *
 * We are trying to avoid an inheritance hierarchy here. We expect there to be many
 * different types of radios, even in the short term. We want a uniform way to interact with them
 * all, but we don't want the readability headaches that go with elaborate inheritance schemes.  Our
 * approach is to define a set of function templates and provide specializations of these templates
 * for each radio type. These template specializations will be in their own header files and be
 * locked away as separate C++ libraries under Bazel. This scheme will also avoid any runtime
 * overhead; the compiler will only include the code that we need in each executable, and there will
 * be no virtual table indirections.
 */
#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "radio/radio.pb.h"
#include "radio/settings.h"
#include "radio/settings.pb.h"
#include "random/random.h"

namespace tvsc::radio {

/**
 * Get the capabilities of the radio module. The returned map gives the valid values for every
 * function of the radio. This function should be specialized for each driver. By default, this
 * template function is undefined.
 */
template <typename DriverT>
std::unordered_map<Function, Value> generate_capabilities_map();

/**
 * Read a single setting from the driver. This function should be specialized for each driver. By
 * default, this template function is undefined.
 */
template <typename DriverT>
DiscreteValue read_setting(DriverT& driver, Function function);

/**
 * Write a single setting to a driver. Only specialize this template if settings can be set
 * independently from each other without adverse effect. If not, specialize the write_settings()
 * function below and do not specialize or define this one. By default, this template function is
 * undefined.
 */
template <typename DriverT>
void write_setting(DriverT& driver, Function function, const DiscreteValue& value);

/**
 * Write some settings to the driver. Assumes that every setting can be set independently.
 * If the settings cannot be set independently, this template function can be specialized for the
 * driver to set them using whatever approach is needed.
 */
template <typename DriverT>
void write_settings(DriverT& driver,
                    const std::unordered_map<Function, DiscreteValue>& pending_settings) {
  for (const auto& entry : pending_settings) {
    write_setting<DriverT>(driver, entry.first, entry.second);
  }
}

/**
 * This template defines an interface for configuring a radio. The specific radio type will be
 * driven by the template parameter. While it is possible to specialize this class template, it
 * should be avoided, if possible. A better approach is to specialize the template functions above
 * to get the appropriate behavior for the radio driver.
 */
template <typename DriverT>
class RadioConfiguration final {
 private:
  DriverT* const driver_;
  const std::unordered_map<Function, Value> capabilities_;
  RadioIdentification identification_;

  std::unordered_map<Function, DiscreteValue> pending_settings_changes_{};

  static RadioIdentification generate_identification(std::string_view name) {
    RadioIdentification identification{};
    identification.set_expanded_id(tvsc::random::generate_random_value<uint64_t>());
    identification.set_id(identification.expanded_id() & static_cast<uint16_t>(0xffff));
    identification.set_name(std::string{name});
    return identification;
  }

 public:
  RadioConfiguration(DriverT& driver, std::string_view name = typeid(DriverT).name())
      : driver_(&driver),
        capabilities_(generate_capabilities_map<DriverT>()),
        identification_(generate_identification(name)) {
    // We know the number of settings at compile time, and we know that number will not change.
    // Because of this, we can reserve the exact number of buckets we need and use a load factor of
    // 1 to save space. Also, since the number of elements is tiny, even if we "degrade" lookups to
    // O(n) with this setup, we will still see good performance.
    pending_settings_changes_.max_load_factor(1.f);
    pending_settings_changes_.reserve(capabilities_.size());
  }

  uint64_t expanded_id() const { return identification_.expanded_id(); }

  uint32_t id() const { return identification_.id(); }
  void set_id(uint32_t id) { identification_.set_id(id); }

  const std::string& name() const { return identification_.name(); }

  const RadioIdentification& identification() const { return identification_; }
  RadioIdentification& identification() { return identification_; }

  Settings settings() const { return {}; }
  Capabilities capabilities() const { return {}; }

  std::vector<Function> get_configurable_functions() const {
    std::vector<Function> functions{};
    functions.reserve(capabilities_.size());
    for (const auto& [key, _] : capabilities_) {
      functions.push_back(key);
    }
    return functions;
  }

  Value get_valid_values(Function function) const { return capabilities_.at(function); }

  DiscreteValue get_value(Function function) const {
    return read_setting<DriverT>(*driver_, function);
  }

  void set_value(Function function, const DiscreteValue& value) {
    if (is_valid_setting(get_valid_values(function), value)) {
      pending_settings_changes_.insert({function, value});
    } else {
      throw std::domain_error("Invalid value for function.");
    }
  }

  void commit_changes() {
    write_settings(*driver_, pending_settings_changes_);
    pending_settings_changes_.clear();
  }

  void abort_changes() { pending_settings_changes_.clear(); }
};

}  // namespace tvsc::radio
