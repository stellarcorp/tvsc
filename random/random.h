#pragma once

#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

namespace tvsc::random {

std::default_random_engine& engine();

inline void set_seed(uint32_t seed) { engine().seed(seed); }

template <typename FirstT, typename SecondT = FirstT,
          std::enable_if_t<std::is_integral<FirstT>::value && std::is_integral<SecondT>::value,
                           bool> = true>
inline auto generate_random_value(FirstT minimum = std::numeric_limits<FirstT>::lowest(),
                                  SecondT maximum = std::numeric_limits<SecondT>::max())
    // TODO(james): Switch to using std::common_type here and similar below.
    -> decltype(minimum + maximum) {
  using ResultT = decltype(minimum + maximum);
  std::uniform_int_distribution dist{static_cast<ResultT>(minimum), static_cast<ResultT>(maximum)};
  return dist(engine());
}

template <typename FirstT, typename SecondT = FirstT,
          std::enable_if_t<std::is_floating_point<FirstT>::value &&
                               std::is_floating_point<SecondT>::value,
                           bool> = true>
inline auto generate_random_value(FirstT minimum = std::numeric_limits<FirstT>::lowest(),
                                  SecondT maximum = std::numeric_limits<SecondT>::max())
    -> decltype(minimum + maximum) {
  using ResultT = decltype(minimum + maximum);
  std::uniform_real_distribution dist{static_cast<ResultT>(minimum), static_cast<ResultT>(maximum)};
  return dist(engine());
}

}  // namespace tvsc::random
