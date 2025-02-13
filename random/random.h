#pragma once

#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

namespace tvsc::random {

std::default_random_engine& engine();

inline void set_seed(uint32_t seed) { engine().seed(seed); }

template <typename ResultT, std::enable_if_t<std::is_integral<ResultT>::value, bool> = true>
inline ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                                     ResultT maximum = std::numeric_limits<ResultT>::max()) {
  std::uniform_int_distribution dist{minimum, maximum};
  return dist(engine());
}

template <typename ResultT, std::enable_if_t<std::is_floating_point<ResultT>::value, bool> = true>
inline ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                                     ResultT maximum = std::numeric_limits<ResultT>::max()) {
  std::uniform_real_distribution dist{minimum, maximum};
  return dist(engine());
}

}  // namespace tvsc::random
