#pragma once

#include <algorithm>
#include <charconv>
#include <initializer_list>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

#include "configuration/allowed_values.h"
#include "configuration/types.h"

namespace tvsc::configuration {

/**
 * Properties are read-only key-value pairs. In a System definition, they are used as configuration 
 * constants.
 */
class Property final {
 private:
  PropertyId property_id_;
  std::string_view name_;
  DiscreteValue value_;

 public:
  template <typename ValueT>
  Property(PropertyId property_id, std::string_view name, ValueT value)
      : property_id_(property_id), name_(name), value_(value) {}

  operator PropertyId() const { return id(); }
  PropertyId id() const { return property_id_; }

  std::string_view name() const { return name_; }

  const DiscreteValue& value() const { return value_; }
};

/**
 * Functions describe configurable capabilities of a system. They can be configured at runtime and
 * are designed to be serializable, including serialized to storage and serialized across a network
 * or radio link. Functions include information about which values are allowed. They do include an
 * actual value.
 */
class Function final {
 private:
  FunctionId function_id_;
  std::string_view name_;
  AllowedValues allowed_values_;

 public:
  Function(FunctionId function_id, std::string_view name, AllowedValues allowed_values)
      : function_id_(function_id), name_(name), allowed_values_(allowed_values) {}

  operator FunctionId() const { return id(); }
  FunctionId id() const { return function_id_; }

  std::string_view name() const { return name_; }

  template <typename ValueT>
  bool is_allowed(const ValueT& value) const {
    return allowed_values_.is_allowed(value);
  }

  const AllowedValues& allowed_values() const { return allowed_values_; }
};

/**
 * Logical definition of a system, including any read-only properties, configurable functions, and
 * any subsystems in a hierarchy. The system definition does not necessarily reflect the literal
 * construction of the system. The actual objects (drivers, etc.) that control these systems do not
 * need to organized exactly as the instances of this class. The system definition represents the
 * logical structure of the system as it might be described to knowledgeable engineers who are
 * unfamiliar with the software.
 *
 * Systems are hierarchical. The subsystems are themselves Systems. This allows for systems to be
 * combined and reused across multiple translation units, though this might make it more difficult
 * to catch misconfigurations.
 *
 * Ids for properties, functions, and subsystems are all local. The values only need to be unique
 * within the same system, and they do not need to be unique between the different types.
 * (Properties can share the same id as Functions, for example.)
 */
class System final {
 private:
  SystemId system_;
  std::string_view name_;
  std::vector<System> subsystems_{};
  std::vector<Property> properties_{};
  std::vector<Function> functions_{};

 public:
  System(SystemId system, std::string_view name) : system_(system), name_(name) {}

  System(SystemId system, std::string_view name, std::initializer_list<System> subsystems)
      : system_(system), name_(name), subsystems_(subsystems.begin(), subsystems.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
  }

  System(SystemId system, std::string_view name, std::initializer_list<Property> properties,
         std::initializer_list<Function> functions)
      : system_(system),
        name_(name),
        properties_(properties.begin(), properties.end()),
        functions_(functions.begin(), functions.end()) {
    std::sort(properties_.begin(), properties_.end());
    std::sort(functions_.begin(), functions_.end());
  }

  System(SystemId system, std::string_view name, std::initializer_list<System> subsystems,
         std::initializer_list<Property> properties, std::initializer_list<Function> functions)
      : system_(system),
        name_(name),
        subsystems_(subsystems.begin(), subsystems.end()),
        properties_(properties.begin(), properties.end()),
        functions_(functions.begin(), functions.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
    std::sort(properties_.begin(), properties_.end());
    std::sort(functions_.begin(), functions_.end());
  }

  operator SystemId() const { return id(); }
  SystemId id() const { return system_; }

  std::string_view name() const { return name_; }

  const std::vector<System>& subsystems() const { return subsystems_; }

  bool has_subsystem(SystemId subsystem_id) const {
    return std::binary_search(subsystems_.begin(), subsystems_.end(), subsystem_id);
  }

  const System* get_subsystem(SystemId subsystem_id) const {
    auto iter{std::lower_bound(subsystems_.begin(), subsystems_.end(), subsystem_id)};
    if (iter == subsystems_.end() || iter->id() != subsystem_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }

  const System* search_subsystems(std::string_view subsystem_id) const {
    SystemId local_id{};
    bool need_recurse{true};

    // Parse out the first number for the subsystem id.
    size_t dot_position{subsystem_id.find_first_of('.')};
    if (dot_position == std::string_view::npos) {
      // No more dots. We have the last number in the address.
      dot_position = subsystem_id.size();
      need_recurse = false;
    }

    auto [next_char, ec] =
        std::from_chars(subsystem_id.data(), subsystem_id.data() + dot_position, local_id);
    if (ec != std::errc()) {
      return nullptr;
    }

    auto iter{std::lower_bound(subsystems_.begin(), subsystems_.end(), local_id)};
    if (iter == subsystems_.end()) {
      return nullptr;
    } else if (iter->id() != local_id) {
      return nullptr;
    } else {
      if (need_recurse) {
        return iter->search_subsystems(
            std::string_view{next_char + 1, subsystem_id.size() - dot_position - 1});
      } else {
        return &(*iter);
      }
    }
  }

  const std::vector<Property>& properties() const { return properties_; }

  bool has_property(PropertyId property_id) const {
    return std::binary_search(properties_.begin(), properties_.end(), property_id);
  }

  const Property* get_property(PropertyId property_id) const {
    auto iter{std::lower_bound(properties_.begin(), properties_.end(), property_id)};
    if (iter == properties_.end() || iter->id() != property_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }

  const std::vector<Function>& functions() const { return functions_; }

  bool has_function(FunctionId function_id) const {
    return std::binary_search(functions_.begin(), functions_.end(), function_id);
  }

  const Function* get_function(FunctionId function_id) const {
    auto iter{std::lower_bound(functions_.begin(), functions_.end(), function_id)};
    if (iter == functions_.end() || iter->id() != function_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }
};

std::string to_string(const Function& function);
std::string to_string(const Property& property);
std::string to_string(const System& system);

}  // namespace tvsc::configuration
