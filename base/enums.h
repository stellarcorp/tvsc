#pragma once

#include <type_traits>

namespace tvsc {

template <typename T>
std::underlying_type_t<T> cast_to_underlying_type(T value) {
  static_assert(std::is_enum_v<T>,
                "This function translates a value from an enum to its underlying integer type. "
                "Applying it to a non-enum is ill-formed.");
  return static_cast<std::underlying_type_t<T>>(value);
}

template <typename T, typename E>
E cast_to_enum(T value) {
  static_assert(std::is_enum_v<E>,
                "This function translates an integer value to an enum that has a compatible "
                "underlying integer type. "
                "Applying it to get a non-enum result is ill-formed.");
  return static_cast<E>(static_cast<std::underlying_type_t<E>>(value));
}

}  // namespace tvsc
