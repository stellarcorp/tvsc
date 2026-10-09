#include <array>
#include <chrono>
#include <cstdint>

#include "base/initializer.h"
#include "hal/board/board.h"
#include "hal/i2c/i2c.h"
#include "hal/pinout/pinout.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;
using namespace tvsc::hal::i2c;
using namespace tvsc::hal::pinout;
using namespace std::chrono_literals;

__attribute__((section(".status.value")))
std::array<std::array<uint8_t, NUM_VALID_I2C_ADDRESSES>, Pinout::NUM_I2C_BUSES>
    discovered_devices{};

void scan_i2c_bus(size_t bus) {
  BoardType& board{BoardType::board()};
  auto& i2c_peripheral{board.mcu().i2c(bus)};
  auto i2c{i2c_peripheral.access()};
  tvsc::hal::i2c::scan_bus(i2c, discovered_devices.at(bus));
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& clock{ClockType::clock()};
  while (true) {
    for (size_t i = 0; i < Pinout::NUM_I2C_BUSES; ++i) {
      scan_i2c_bus(i);
      clock.wait(25ms);
    }
    clock.wait(5s);
  }
}
