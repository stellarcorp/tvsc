/**
 * Script to scan all I2C buses and report the addresses of any discovered devices into an array.
 *
 * This script blinks the debug LED when it is scanning a bus. Note that the delays built into these
 * blinks are vastly longer than the time it takes to scan the buses.
 */
#include <array>
#include <chrono>
#include <cstdint>

#include "base/constexpr_for.h"
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

template <size_t BUS>
void scan_i2c_bus() {
  BoardType& board{BoardType::board()};
  auto& i2c_peripheral{board.mcu().i2c<BUS>()};
  auto i2c{i2c_peripheral.access()};
  tvsc::hal::i2c::scan_bus(i2c, discovered_devices[BUS]);
}

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};
  auto& led_peripheral{board.debug_led()};

  while (true) {
    tvsc::constexpr_for<0, Pinout::NUM_I2C_BUSES>([&](auto i) {
      {
        auto led{led_peripheral.access()};
        led.on();
        scan_i2c_bus<i>();
        clock.wait(100ms);
        led.off();
      }
      clock.wait(250ms);
    });
    clock.wait(5s);
  }
}
