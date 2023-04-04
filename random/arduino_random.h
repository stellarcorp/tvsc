#pragma once

#include <cstdint>

#include "Arduino.h"
#include "Entropy.h"

namespace tvsc::random {

inline void set_seed(uint32_t seed) { randomSeed(seed); }

inline void initialize_seed() {
  Entropy.Initialize();
  set_seed(Entropy.random());
}

inline uint32_t generate_entropy() {
  return static_cast<uint32_t>(::random());
}

}  // namespace tvsc::random
