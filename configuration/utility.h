#pragma once

#include <cstdint>
#include <type_traits>

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

}  // namespace tvsc::configuration
