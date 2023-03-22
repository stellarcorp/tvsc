#pragma once

#include <Arduino.h>

namespace tvsc::random {

template <>
inline void set_seed(uint32_t seed) {
  randomSeed(seed);
}

template <>
inline int32_t generate_random_value(int32_t minimum_value, int32_t maximum_value) {
  return ::random(minimum_value, maximum_value);
}

template <>
inline uint32_t generate_random_value(uint32_t minimum_value, uint32_t maximum_value) {
  return ::random(minimum_value, maximum_value);
}

template <>
inline uint8_t generate_random_value(uint8_t minimum, uint8_t maximum) {
  return generate_random_value<int32_t>(minimum, maximum);
}

template <>
inline uint64_t generate_random_value(uint64_t minimum, uint64_t maximum) {
  uint64_t result = generate_random_value<uint32_t>();
  result = result << 32;
  result |= generate_random_value<uint32_t>();
  result = result % (maximum - minimum);
  result += minimum;
  return result;
}

}  // namespace tvsc::random
