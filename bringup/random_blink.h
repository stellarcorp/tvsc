#include <chrono>
#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "hal/time/clock.h"
#include "random/random.h"

namespace tvsc::bringup {

template <typename ClockType>
tvsc::scheduler::Task<ClockType> blink_randomly(
    tvsc::hal::gpio::GpioPeripheral& gpio_peripheral, tvsc::hal::gpio::Pin pin) {
  auto gpio{gpio_peripheral.access()};
  gpio.set_pin_mode(pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
  gpio.write_pin(pin, 0);

  while (true) {
    gpio.toggle_pin(pin);
    const std::chrono::milliseconds delay_ms{tvsc::random::generate_random_value(5U, 1000U)};
    co_yield delay_ms;
  }
}

}  // namespace tvsc::bringup
