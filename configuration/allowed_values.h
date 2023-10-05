#pragma once

#include <algorithm>
#include <initializer_list>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "configuration/utility.h"

namespace tvsc::configuration {

template <typename T>
using ValueRange = std::pair<T, T>;

class AllowedValues final {
 public:
  using DiscreteValueT = std::variant<int32_t, int64_t, float, double>;
  using RangedValueT =
      std::variant<ValueRange<int32_t>, ValueRange<int64_t>, ValueRange<float>, ValueRange<double>>;

 private:
  std::vector<DiscreteValueT> enumerated_;
  std::vector<RangedValueT> ranged_;

 public:
  AllowedValues() : enumerated_(), ranged_() {}

  AllowedValues(std::initializer_list<int32_t> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()), ranged_() {}

  AllowedValues(std::initializer_list<int64_t> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()), ranged_() {}

  AllowedValues(std::initializer_list<float> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()), ranged_() {}

  AllowedValues(std::initializer_list<double> allowed_values)
      : enumerated_(allowed_values.begin(), allowed_values.end()), ranged_() {}

  AllowedValues(std::initializer_list<ValueRange<int32_t>> allowed_values)
      : enumerated_(), ranged_(allowed_values.begin(), allowed_values.end()) {}

  AllowedValues(std::initializer_list<ValueRange<int64_t>> allowed_values)
      : enumerated_(), ranged_(allowed_values.begin(), allowed_values.end()) {}

  AllowedValues(std::initializer_list<ValueRange<float>> allowed_values)
      : enumerated_(), ranged_(allowed_values.begin(), allowed_values.end()) {}

  AllowedValues(std::initializer_list<ValueRange<double>> allowed_values)
      : enumerated_(), ranged_(allowed_values.begin(), allowed_values.end()) {}

  template <typename EnumT>
  AllowedValues(std::initializer_list<EnumT> allowed_values)
      : enumerated_(as_int(allowed_values.begin()), as_int(allowed_values.end())), ranged_() {}

  AllowedValues(const AllowedValues& rhs) = default;
  AllowedValues(AllowedValues&& rhs) = default;
  AllowedValues& operator=(const AllowedValues& rhs) = default;
  AllowedValues& operator=(AllowedValues&& rhs) = default;

  bool is_allowed(int32_t value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<int32_t>(&v) != nullptr && *std::get_if<int32_t>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<int32_t>>(&v) != nullptr &&
          std::get_if<ValueRange<int32_t>>(&v)->first <= value &&
          std::get_if<ValueRange<int32_t>>(&v)->second >= value) {
        return true;
      }
    }
    return is_allowed(static_cast<int64_t>(value));
  }

  bool is_allowed(int64_t value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<int64_t>(&v) != nullptr && *std::get_if<int64_t>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<int64_t>>(&v) != nullptr &&
          std::get_if<ValueRange<int64_t>>(&v)->first <= value &&
          std::get_if<ValueRange<int64_t>>(&v)->second >= value) {
        return true;
      }
    }
    return false;
  }

  bool is_allowed(float value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<float>(&v) != nullptr && *std::get_if<float>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<float>>(&v) != nullptr &&
          std::get_if<ValueRange<float>>(&v)->first <= value &&
          std::get_if<ValueRange<float>>(&v)->second > value) {
        return true;
      }
    }
    return is_allowed(static_cast<double>(value));
  }

  bool is_allowed(double value) const {
    for (const auto& v : enumerated_) {
      if (std::get_if<double>(&v) != nullptr && *std::get_if<double>(&v) == value) {
        return true;
      }
    }
    for (const auto& v : ranged_) {
      if (std::get_if<ValueRange<double>>(&v) != nullptr &&
          std::get_if<ValueRange<double>>(&v)->first <= value &&
          std::get_if<ValueRange<double>>(&v)->second > value) {
        return true;
      }
    }
    return false;
  }

  template <typename ValueT>
  bool is_allowed(ValueT value) const {
    static_assert(std::is_enum<ValueT>::value);
    for (const auto& v : enumerated_) {
      if (std::get_if<std::underlying_type_t<ValueT>>(&v) != nullptr &&
          *std::get_if<std::underlying_type_t<ValueT>>(&v) ==
              std::underlying_type_t<ValueT>(value)) {
        return true;
      }
    }
    return false;
  }

  constexpr const std::vector<DiscreteValueT>& enumerated_values() const { return enumerated_; }
  constexpr const std::vector<RangedValueT>& ranged_values() const { return ranged_; }
};

std::string to_string(const AllowedValues& values);
std::string to_string(const AllowedValues::RangedValueT& value);
std::string to_string(const AllowedValues::DiscreteValueT& value);

template <typename T>
inline std::string to_string(const ValueRange<T>& value) {
  using std::to_string;
  std::string result{};
  result.append("[")
      .append(to_string(value.first))
      .append(", ")
      .append(to_string(value.second))
      .append("]");
  return result;
}

template <>
inline std::string to_string(const ValueRange<float>& value) {
  using std::to_string;
  std::string result{};
  result.append("[")
      .append(to_string(value.first))
      .append(", ")
      .append(to_string(value.second))
      .append(")");
  return result;
}

template <>
inline std::string to_string(const ValueRange<double>& value) {
  using std::to_string;
  std::string result{};
  result.append("[")
      .append(to_string(value.first))
      .append(", ")
      .append(to_string(value.second))
      .append(")");
  return result;
}

}  // namespace tvsc::configuration
