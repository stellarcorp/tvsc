#pragma once

#include <limits>
#include <random>

namespace tvsc::random {

std::mt19937& engine() {
  static std::mt19937 engine{};
  return engine;
}

template <typename ResultT>
ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                              ResultT maximum = std::numeric_limits<ResultT>::max());

template <>
uint8_t generate_random_value(uint8_t minimum, uint8_t maximum) {
  std::uniform_int_distribution<> distribution(minimum, maximum);
  return distribution(engine());
}

template <>
uint64_t generate_random_value(uint64_t minimum, uint64_t maximum) {
  std::uniform_int_distribution<> distribution(minimum, maximum);
  return distribution(engine());
}

}  // namespace tvsc::random
