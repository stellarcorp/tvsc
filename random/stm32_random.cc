#include <cstdint>
#include <random>

#include "hal/board/board.h"
#include "hal/random/rng.h"
#include "random.h"

namespace tvsc::random {

std::default_random_engine& engine() {
  static std::default_random_engine engine{[]() {
    tvsc::hal::board::Board& board{tvsc::hal::board::Board::board()};
    tvsc::hal::random::RngPeripheral& device{board.rng()};
    // Note that this call can take several milliseconds. It makes multiple calls into the STM32 HAL
    // which can each block for multiple milliseconds, usually 2ms each.
    tvsc::hal::random::Rng rng{device.access()};
    return rng();
  }()};
  return engine;
}

}  // namespace tvsc::random
