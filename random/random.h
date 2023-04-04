#pragma once

#include <cstdint>
#include <limits>

namespace tvsc::random {

inline void initialize_seed();
inline void set_seed(uint32_t seed);
inline uint32_t generate_entropy();

template <typename ResultT>
inline ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                                     ResultT maximum = std::numeric_limits<ResultT>::max()) {
  uint32_t result = generate_entropy();
  result = result % (maximum - minimum);
  result += minimum;
  return static_cast<ResultT>(result);
}

template <>
inline int64_t generate_random_value(int64_t minimum, int64_t maximum) {
  uint64_t result = generate_entropy();
  result = result << 32;
  result |= generate_entropy();
  result = result % (maximum - minimum);
  return static_cast<int64_t>(result) + minimum;
}

template <>
inline uint64_t generate_random_value(uint64_t minimum, uint64_t maximum) {
  uint64_t result = generate_entropy();
  result = result << 32;
  result |= generate_entropy();
  result = result % (maximum - minimum);
  result += minimum;
  return result;
}

}  // namespace tvsc::random

// These headers define actual specializations of the templates above that are appropriate to the
// target platform.
#ifdef ARDUINO
#include "random/arduino_random.h"
#else
#include "random/std_cpp_random.h"
#endif
