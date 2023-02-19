#pragma once

#include <ratio>

namespace tvsc {

/**
 * Express a number in a unit where the "unit" is just a std::ratio.
 *
 * TODO(james): Expand this idea to include real units of time, distance, frequency, power, etc.
 */
template <typename UnitT, typename T>
constexpr float in_unit(T a) {
  return a * UnitT::num / static_cast<float>(UnitT::den);
}

}  // namespace tvsc
