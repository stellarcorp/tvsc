#pragma once

#include <type_traits>

namespace tvsc {

template <typename E>
  requires(std::is_enum_v<E>)
[[nodiscard]] inline constexpr std::underlying_type_t<E> cast_to_underlying_type(E value) noexcept {
  return static_cast<std::underlying_type_t<E>>(value);
}

template <typename T, typename E>
  requires(std::is_enum_v<E>)
[[nodiscard]] inline constexpr E cast_to_enum(T value) noexcept {
  return static_cast<E>(static_cast<std::underlying_type_t<E>>(value));
}

}  // namespace tvsc
