#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"
#include "random/random.h"

namespace tvsc::hal::bringup {

template <uint64_t DURATION_MS = 365UL * 24 * 60 * 60 * 1000 /* one year in milliseconds */>
scheduler::Task blink_randomly(time::Clock& clock, gpio::GpioPeripheral& gpio_peripheral,
                               gpio::Pin pin) {
  auto gpio{gpio_peripheral.access()};

  gpio.set_pin_mode(pin, gpio::PinMode::OUTPUT_PUSH_PULL);
  const uint64_t stop_time_ms{clock.current_time_millis() + DURATION_MS};

  gpio.write_pin(pin, 0);
  while (clock.current_time_millis() < stop_time_ms) {
    gpio.toggle_pin(pin);
    const uint32_t delay_ms{tvsc::random::generate_random_value(5U, 1000U)};
    co_yield 1000 * (clock.current_time_millis() + delay_ms);
  }
  gpio.write_pin(pin, 0);
  co_return;
}

}  // namespace tvsc::hal::bringup
