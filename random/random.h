#pragma once

#include <limits>
#include <random>

namespace tvsc::random {

template <typename ResultT>
ResultT generate_random_value(ResultT minimum = std::numeric_limits<ResultT>::lowest(),
                              ResultT maximum = std::numeric_limits<ResultT>::max());

template <>
uint8_t generate_random_value(uint8_t minimum, uint8_t maximum) {
  static std::random_device random_device{};
  static std::default_random_engine generator{random_device()};
  std::uniform_int_distribution<> distribution(minimum, maximum);
  return distribution(generator);
}

template <>
uint64_t generate_random_value(uint64_t minimum, uint64_t maximum) {
  static std::random_device random_device{};
  static std::default_random_engine generator{random_device()};
  std::uniform_int_distribution<> distribution(minimum, maximum);
  return distribution(generator);
}

}  // namespace tvsc::random
