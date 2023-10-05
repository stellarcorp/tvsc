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
#include "configuration/utility.h"

namespace tvsc::configuration {

class Function final {
 private:
  FunctionId function_id_;

  AllowedValues allowed_values_;

 public:
  template <typename ValueT>
  Function(FunctionId function_id, std::initializer_list<ValueT> allowed_values)
      : function_id_(function_id), allowed_values_(allowed_values) {}

  Function(FunctionId function_id, AllowedValues allowed_values)
      : function_id_(function_id), allowed_values_(allowed_values) {}

  Function(const Function& rhs) = default;
  Function(Function&& rhs) = default;
  Function& operator=(const Function& rhs) = default;
  Function& operator=(Function&& rhs) = default;

  /**
   * Functions are identified by their function_id. These methods
   * define that identification scheme.
   *
   * Also, functions are ordered for faster lookup. These methods define that ordering.
   */
  operator FunctionId() const { return identifier(); }

  bool operator<(const Function& rhs) const { return identifier() < rhs.identifier(); }
  bool operator==(const Function& rhs) const { return identifier() == rhs.identifier(); }

  FunctionId function_id() const { return function_id_; }

  FunctionId identifier() const { return function_id(); }

  template <typename ValueT>
  bool is_allowed(const ValueT& value) const {
    return allowed_values_.is_allowed(value);
  }

  const AllowedValues& allowed_values() const { return allowed_values_; }
};

class Component final {
 private:
  ComponentId component_id_;
  std::vector<Function> functions_;

 public:
  Component(ComponentId component_id, std::initializer_list<Function> functions)
      : component_id_(component_id), functions_(functions.begin(), functions.end()) {
    // Note that std::sort() is starting in C++20.
    std::sort(functions_.begin(), functions_.end());
  }

  operator ComponentId() const { return identifier(); }
  ComponentId identifier() const { return component_id_; }

  const std::vector<Function>& functions() const { return functions_; }

  bool has_function(FunctionId function_id) const {
    return std::binary_search(functions_.begin(), functions_.end(), function_id);
  }

  const Function* search(FunctionId function_id) const {
    auto iter{std::lower_bound(functions_.begin(), functions_.end(), function_id)};
    if (iter == functions_.end() || iter->identifier() != function_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }
};

class System final {
 private:
  SystemId system_;
  std::vector<System> subsystems_{};
  std::vector<Component> components_{};

 public:
  System(SystemId system) : system_(system) {}

  System(SystemId system, std::initializer_list<System> subsystems)
      : system_(system), subsystems_(subsystems.begin(), subsystems.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
  }

  System(SystemId system, std::initializer_list<Component> components)
      : system_(system), components_(components.begin(), components.end()) {
    std::sort(components_.begin(), components_.end());
  }

  System(SystemId system, std::initializer_list<System> subsystems,
         std::initializer_list<Component> components)
      : system_(system),
        subsystems_(subsystems.begin(), subsystems.end()),
        components_(components.begin(), components.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
    std::sort(components_.begin(), components_.end());
  }

  operator SystemId() const { return identifier(); }
  SystemId identifier() const { return system_; }

  const std::vector<System>& subsystems() const { return subsystems_; }

  bool has_subsystem(SystemId subsystem_id) const {
    return std::binary_search(subsystems_.begin(), subsystems_.end(), subsystem_id);
  }

  const System* search_subsystems(SystemId subsystem_id) const {
    auto iter{std::lower_bound(subsystems_.begin(), subsystems_.end(), subsystem_id)};
    if (iter == subsystems_.end() || iter->identifier() != subsystem_id) {
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
    } else if (iter->identifier() != local_id) {
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

  const std::vector<Component>& components() const { return components_; }

  bool has_component(ComponentId component_id) const {
    return std::binary_search(components_.begin(), components_.end(), component_id);
  }

  const Component* search_components(ComponentId component_id) const {
    auto iter{std::lower_bound(components_.begin(), components_.end(), component_id)};
    if (iter == components_.end() || iter->identifier() != component_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }
};

std::string to_string(const Function& function);
std::string to_string(const Component& component);
std::string to_string(const System& system);

}  // namespace tvsc::configuration
