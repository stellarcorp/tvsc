#pragma once

#include <algorithm>
#include <initializer_list>
#include <variant>
#include <vector>

#include "configuration/utility.h"

namespace tvsc::configuration {

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

}  // namespace tvsc::configuration
