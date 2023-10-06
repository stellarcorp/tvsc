#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>

namespace tvsc::configuration {

using SystemId = uint8_t;
using FunctionId = uint8_t;

template <typename T>
using ValueRange = std::pair<T, T>;

using DiscreteValue = std::variant<int32_t, int64_t, float, double>;
using RangedValue =
    std::variant<ValueRange<int32_t>, ValueRange<int64_t>, ValueRange<float>, ValueRange<double>>;

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

}  // namespace tvsc::configuration
