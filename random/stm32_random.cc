#include <cstdint>
#include <random>

#include "hal/random/rng.h"
#include "hal/random/stm32xxxx_rng.h"
#include "random.h"

namespace tvsc::random {

std::default_random_engine& engine() {
  static std::default_random_engine engine{[]() {
    tvsc::hal::random::RngStm32xxxx device{};
    tvsc::hal::random::Rng rng{device.access()};
    return rng();
  }()};
  return engine;
}

}  // namespace tvsc::random
