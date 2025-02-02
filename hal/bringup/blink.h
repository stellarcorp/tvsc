#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

namespace tvsc::hal::bringup {

template <uint64_t DURATION_MS = 365UL * 24 * 60 * 60 * 1000 /* one year in milliseconds */>
scheduler::Task blink(time::Clock& clock, gpio::Gpio& gpio, gpio::Pin pin,
                      uint64_t delay_ms = 500) {
  const PowerToken gpio_power{gpio.enable()};

  gpio.set_pin_mode(pin, gpio::PinMode::OUTPUT_PUSH_PULL);
  const uint64_t stop_time_ms{clock.current_time_millis() + DURATION_MS};

  gpio.write_pin(pin, 0);
  while (clock.current_time_millis() < stop_time_ms) {
    gpio.toggle_pin(pin);
    co_yield 1000 * (clock.current_time_millis() + delay_ms);
  }
  gpio.write_pin(pin, 0);
  co_return;
}

}  // namespace tvsc::hal::bringup
