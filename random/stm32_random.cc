#include <cstdint>
#include <random>

#include "hal/random/stm32xxxx_rng.h"
#include "random.h"

namespace tvsc::random {

std::default_random_engine& engine() {
  static std::default_random_engine engine{[]() {
    tvsc::hal::random::RngStm32xxxx device{};
    tvsc::hal::EnableLock enabler{device.enable()};
    return device();
  }()};
  return engine;
}

}  // namespace tvsc::random
