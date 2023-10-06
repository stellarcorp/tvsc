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

class Function final {
 private:
  FunctionId function_id_;
  std::string_view name_;
  AllowedValues allowed_values_;

 public:
  template <typename ValueT>
  Function(FunctionId function_id, std::string_view name,
           std::initializer_list<ValueT> allowed_values)
      : function_id_(function_id), name_(name), allowed_values_(allowed_values) {}

  Function(FunctionId function_id, std::string_view name, AllowedValues allowed_values)
      : function_id_(function_id), name_(name), allowed_values_(allowed_values) {}

  Function(const Function& rhs) = default;
  Function(Function&& rhs) = default;
  Function& operator=(const Function& rhs) = default;
  Function& operator=(Function&& rhs) = default;

  operator FunctionId() const { return id(); }
  FunctionId id() const { return function_id_; }

  template <typename ValueT>
  bool is_allowed(const ValueT& value) const {
    return allowed_values_.is_allowed(value);
  }

  const AllowedValues& allowed_values() const { return allowed_values_; }
};

class System final {
 private:
  SystemId system_;
  std::string_view name_;
  std::vector<System> subsystems_{};
  std::vector<Function> functions_;

 public:
  System(SystemId system, std::string_view name) : system_(system), name_(name) {}

  System(SystemId system, std::string_view name, std::initializer_list<System> subsystems)
      : system_(system), name_(name), subsystems_(subsystems.begin(), subsystems.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
  }

  System(SystemId system, std::string_view name, std::initializer_list<Function> functions)
      : system_(system), name_(name), functions_(functions.begin(), functions.end()) {
    std::sort(functions_.begin(), functions_.end());
  }

  System(SystemId system, std::string_view name, std::initializer_list<System> subsystems,
         std::initializer_list<Function> functions)
      : system_(system),
        name_(name),
        subsystems_(subsystems.begin(), subsystems.end()),
        functions_(functions.begin(), functions.end()) {
    std::sort(subsystems_.begin(), subsystems_.end());
    std::sort(functions_.begin(), functions_.end());
  }

  operator SystemId() const { return id(); }
  SystemId id() const { return system_; }

  const std::vector<System>& subsystems() const { return subsystems_; }

  bool has_subsystem(SystemId subsystem_id) const {
    return std::binary_search(subsystems_.begin(), subsystems_.end(), subsystem_id);
  }

  const System* search_subsystems(SystemId subsystem_id) const {
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

  const std::vector<Function>& functions() const { return functions_; }

  bool has_function(FunctionId function_id) const {
    return std::binary_search(functions_.begin(), functions_.end(), function_id);
  }

  const Function* search_functions(FunctionId function_id) const {
    auto iter{std::lower_bound(functions_.begin(), functions_.end(), function_id)};
    if (iter == functions_.end() || iter->id() != function_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }
};

std::string to_string(const Function& function);
std::string to_string(const System& system);

}  // namespace tvsc::configuration
