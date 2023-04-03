#pragma once

#include <random>

namespace tvsc::random {

inline std::mt19937& engine() {
  static std::mt19937 engine{};
  return engine;
}

template <>
inline void set_seed<uint32_t>(uint32_t seed) {
  engine().seed(seed);
}

template <>
inline uint8_t generate_random_value(uint8_t minimum, uint8_t maximum) {
  std::uniform_int_distribution<> distribution(minimum, maximum);
  return distribution(engine());
}

template <>
inline uint64_t generate_random_value(uint64_t minimum, uint64_t maximum) {
  std::uniform_int_distribution<> distribution(minimum, maximum);
  return distribution(engine());
}

inline void initialize_seed() {
  std::random_device device{};
  set_seed(device());
}

}  // namespace tvsc::random
