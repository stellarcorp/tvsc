#pragma once

#include <cstdint>
#include <limits>
#include <random>

namespace tvsc::random {

std::default_random_engine& engine();

inline void set_seed(uint32_t seed) { engine().seed(seed); }

template <typename ResultT>
inline ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                                     ResultT maximum = std::numeric_limits<ResultT>::max()) {
  std::uniform_int_distribution dist{minimum, maximum};
  return dist(engine());
}

}  // namespace tvsc::random
