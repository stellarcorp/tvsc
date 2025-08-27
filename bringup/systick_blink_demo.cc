#include <chrono>
#include <cstdint>
#include <limits>

#include "base/initializer.h"
#include "flags/flags.h"
#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "profile/profile.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  tvsc::profile::profiler_start();

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};
  const auto completion_time{clock.current_time() + std::chrono::seconds{FLAGS_run_duration_sec}};

  const auto delay{500ms};

  tvsc::hal::gpio::GpioPeripheral& gpio_peripheral{board.gpio<BoardType::DEBUG_LED_PORT>()};
  tvsc::hal::gpio::Gpio gpio{gpio_peripheral.access()};
  tvsc::hal::gpio::PinNumber pin{BoardType::DEBUG_LED_PIN};

  gpio.set_pin_mode(pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);

  while (clock.current_time() < completion_time) {
    gpio.toggle_pin(pin);

    const auto change_time{clock.current_time() + delay};
    while (clock.current_time() < change_time) {
      // Busy loop to keep dependencies simple. This avoids using the scheduler and avoids
      // sleep/stop modes used when using the sleep methods of the clock.
      for (int i = 0; i < 1000; ++i) {
        // We use this inside for loop to avoid calling clock.current_time() so often. This reduces
        // overhead in simulation.
      }
    }
  }

  tvsc::profile::profiler_stop();

  return 0;
}
