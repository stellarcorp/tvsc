#pragma once

#include <algorithm>
#include <charconv>
#include <initializer_list>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace tvsc::configuration {

using SystemId = uint8_t;
using ComponentId = uint8_t;
using FunctionId = uint8_t;

// Note that many types and functions related to settings can be made 'constexpr' in C++20. The
// settings package makes heavy use of std::vector. This class gets constexpr constructors,
// iterators and other enabling features starting in C++20.
#if __cplusplus >= 202000
#define CONSTEXPR_SETTINGS constexpr
#else
#define CONSTEXPR_SETTINGS
#endif

template <typename EnumType>
constexpr auto as_int(EnumType value) {
  return static_cast<std::underlying_type_t<EnumType>>(value);
}

template <typename EnumType>
constexpr auto as_int(EnumType* value) {
  return reinterpret_cast<std::underlying_type_t<EnumType>*>(value);
}

template <typename EnumType>
constexpr auto as_int(const EnumType* value) {
  return reinterpret_cast<const std::underlying_type_t<EnumType>*>(value);
}

class AllowedValues final {
 public:
  using StoredT = std::variant<int32_t, int64_t, float, double>;

 private:
  std::vector<StoredT> enumerated_{};

 public:
  constexpr AllowedValues() = default;

  CONSTEXPR_SETTINGS AllowedValues(std::initializer_list<int32_t> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()) {}

  CONSTEXPR_SETTINGS AllowedValues(std::initializer_list<int64_t> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()) {}

  CONSTEXPR_SETTINGS AllowedValues(std::initializer_list<float> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()) {}

  CONSTEXPR_SETTINGS AllowedValues(std::initializer_list<double> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()) {}

  template <typename EnumT>
  CONSTEXPR_SETTINGS AllowedValues(std::initializer_list<EnumT> allowed_values)
      : enumerated_(as_int(allowed_values.begin()), as_int(allowed_values.end())) {}

  AllowedValues(const AllowedValues& rhs) = default;
  AllowedValues(AllowedValues&& rhs) = default;
  AllowedValues& operator=(const AllowedValues& rhs) = default;
  AllowedValues& operator=(AllowedValues&& rhs) = default;

  CONSTEXPR_SETTINGS bool is_allowed(int32_t value) const {
    for (const auto& v : enumerated_) {
      if (std::get<int32_t>(v) == value) return true;
    }
    return false;
  }

  CONSTEXPR_SETTINGS bool is_allowed(int64_t value) const {
    for (const auto& v : enumerated_) {
      if (std::get<int64_t>(v) == value) return true;
    }
    return false;
  }

  CONSTEXPR_SETTINGS bool is_allowed(float value) const {
    for (const auto& v : enumerated_) {
      if (std::get<float>(v) == value) return true;
    }
    return false;
  }

  CONSTEXPR_SETTINGS bool is_allowed(double value) const {
    for (const auto& v : enumerated_) {
      if (std::get<double>(v) == value) return true;
    }
    return false;
  }

  template <typename ValueT>
  CONSTEXPR_SETTINGS bool is_allowed(ValueT value) const {
    static_assert(std::is_enum<ValueT>::value);
    for (const auto& v : enumerated_) {
      if (std::get<std::underlying_type_t<ValueT>>(v) == std::underlying_type_t<ValueT>(value))
        return true;
    }
    return false;
  }

  constexpr const std::vector<StoredT>& enumerated_values() const { return enumerated_; }
};

class Function final {
 private:
  FunctionId function_id_;

  AllowedValues allowed_values_;

 public:
  template <typename ValueT>
  CONSTEXPR_SETTINGS Function(FunctionId function_id, std::initializer_list<ValueT> allowed_values)
      : function_id_(function_id), allowed_values_(allowed_values) {}

  CONSTEXPR_SETTINGS Function(FunctionId function_id, AllowedValues allowed_values)
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
  constexpr operator FunctionId() const { return identifier(); }

  constexpr bool operator<(const Function& rhs) const { return identifier() < rhs.identifier(); }
  constexpr bool operator==(const Function& rhs) const { return identifier() == rhs.identifier(); }

  constexpr FunctionId function_id() const { return function_id_; }

  constexpr FunctionId identifier() const { return function_id(); }

  template <typename ValueT>
  CONSTEXPR_SETTINGS bool is_allowed(const ValueT& value) const {
    return allowed_values_.is_allowed(value);
  }

  constexpr const AllowedValues& allowed_values() const { return allowed_values_; }
};

class Component final {
 private:
  ComponentId component_id_;
  std::vector<Function> functions_;

 public:
  CONSTEXPR_SETTINGS Component(ComponentId component_id, std::initializer_list<Function> functions)
      : component_id_(component_id), functions_(functions.begin(), functions.end()) {
    // Note that std::sort() is constexpr starting in C++20.
    std::sort(functions_.begin(), functions_.end());
  }

  constexpr operator ComponentId() const { return identifier(); }
  constexpr ComponentId identifier() const { return component_id_; }

  CONSTEXPR_SETTINGS const std::vector<Function>& functions() const { return functions_; }

  CONSTEXPR_SETTINGS bool has_function(FunctionId function_id) const {
    return std::binary_search(functions_.begin(), functions_.end(), function_id);
  }

  CONSTEXPR_SETTINGS const Function* search(FunctionId function_id) const {
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
  CONSTEXPR_SETTINGS System(SystemId system) : system_(system) {}

  CONSTEXPR_SETTINGS System(SystemId system, std::initializer_list<System> subsystems)
      : system_(system), subsystems_(subsystems.begin(), subsystems.end()) {
    // Note that std::sort() is constexpr starting in C++20.
    std::sort(subsystems_.begin(), subsystems_.end());
  }

  CONSTEXPR_SETTINGS System(SystemId system, std::initializer_list<Component> components)
      : system_(system), components_(components.begin(), components.end()) {
    // Note that std::sort() is constexpr starting in C++20.
    std::sort(components_.begin(), components_.end());
  }

  CONSTEXPR_SETTINGS System(SystemId system, std::initializer_list<System> subsystems,
                            std::initializer_list<Component> components)
      : system_(system),
        subsystems_(subsystems.begin(), subsystems.end()),
        components_(components.begin(), components.end()) {
    // Note that std::sort() is constexpr starting in C++20.
    std::sort(subsystems_.begin(), subsystems_.end());
    std::sort(components_.begin(), components_.end());
  }

  constexpr operator SystemId() const { return identifier(); }
  constexpr SystemId identifier() const { return system_; }

  CONSTEXPR_SETTINGS const std::vector<System>& subsystems() const { return subsystems_; }

  CONSTEXPR_SETTINGS bool has_subsystem(SystemId subsystem_id) const {
    return std::binary_search(subsystems_.begin(), subsystems_.end(), subsystem_id);
  }

  CONSTEXPR_SETTINGS const System* search_subsystems(SystemId subsystem_id) const {
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

  CONSTEXPR_SETTINGS const std::vector<Component>& components() const { return components_; }

  CONSTEXPR_SETTINGS bool has_component(ComponentId component_id) const {
    return std::binary_search(components_.begin(), components_.end(), component_id);
  }

  CONSTEXPR_SETTINGS const Component* search_components(ComponentId component_id) const {
    auto iter{std::lower_bound(components_.begin(), components_.end(), component_id)};
    if (iter == components_.end() || iter->identifier() != component_id) {
      return nullptr;
    } else {
      return &(*iter);
    }
  }
};

std::string to_string(const AllowedValues& values);
std::string to_string(const Function& function);
std::string to_string(const Component& component);
std::string to_string(const System& system);

}  // namespace tvsc::configuration
