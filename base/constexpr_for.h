#pragma once

#include <type_traits>

namespace tvsc {

/**
 * Utility function to iterate through integral values, calling a functional
 * (usually a lambda) for each value in the range. The loop unrolls at compile-time, allowing
 * execution over fixed integral values. The functionals themselves get called at runtime in the
 * normal order of execution.
 *
 * The primary motivation for this utility is to avoid code duplication. Throughout the system,
 * there are many functions templated on an integer value. For example, the accessors to the various
 * buses (I2C, CAN, SPI, etc) usually follow this pattern. This utility provides a way to call those
 * templated functions for all possible values without requiring a duplicate form of the accessor
 * that accepts the index as a runtime parameter.
 */
template <auto Start, auto End, auto Inc = 1, class F>
constexpr void constexpr_for(F&& f) {
  if constexpr (Start < End) {
    f(std::integral_constant<decltype(Start), Start>{});
    constexpr_for<Start + Inc, End, Inc>(std::forward<F>(f));
  }
}

}  // namespace tvsc
