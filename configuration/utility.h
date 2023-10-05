#pragma once

#include <cstdint>
#include <type_traits>

namespace tvsc::configuration {

using SystemId = uint8_t;
using ComponentId = uint8_t;
using FunctionId = uint8_t;

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
