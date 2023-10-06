#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace tvsc::configuration {

using SystemId = uint8_t;
using PropertyId = uint8_t;
using FunctionId = uint8_t;

template <typename T>
using ValueRange = std::pair<T, T>;

using DiscreteValue = std::variant<int32_t, int64_t, float, double, std::string_view>;
using RangedValue =
    std::variant<ValueRange<int32_t>, ValueRange<int64_t>, ValueRange<float>, ValueRange<double>>;

std::string to_string(const RangedValue& value);
std::string to_string(const DiscreteValue& value);

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

template <typename EnumType>
inline constexpr auto as_int(EnumType value) {
  return static_cast<std::underlying_type_t<EnumType>>(value);
}

template <typename EnumType>
inline constexpr auto as_int(EnumType* value) {
  return reinterpret_cast<std::underlying_type_t<EnumType>*>(value);
}

template <typename EnumType>
inline constexpr auto as_int(const EnumType* value) {
  return reinterpret_cast<const std::underlying_type_t<EnumType>*>(value);
}

}  // namespace tvsc::configuration
