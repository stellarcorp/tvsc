#pragma once

#include <cstdint>
#include <random>

namespace tvsc::random {

namespace internal {

inline std::default_random_engine& engine() {
  static std::random_device device{};
  static std::default_random_engine engine{device()};
  return engine;
}

}  // namespace internal

inline void set_seed(uint32_t seed) { internal::engine().seed(seed); }

inline uint32_t generate_entropy() {
  std::uniform_int_distribution<uint32_t> distribution{};
  return distribution(internal::engine());
}

inline void initialize_seed() {
  // Always initialized on first use.
}

}  // namespace tvsc::random
