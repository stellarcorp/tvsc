#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/scheduler/scheduler.h"
#include "hal/scheduler/task.h"
#include "hal/time/clock.h"

namespace tvsc::hal::bringup {

template <typename ClockType,
          uint64_t DURATION_MS = 365UL * 24 * 60 * 60 * 1000 /* one year in milliseconds */>
scheduler::Task<ClockType> blink(ClockType& clock, gpio::GpioPeripheral& gpio_peripheral,
                                 gpio::Pin pin, uint64_t delay_ms = 500) {
  const std::chrono::milliseconds delay{delay_ms};
  gpio::Gpio gpio{gpio_peripheral.access()};

  gpio.set_pin_mode(pin, gpio::PinMode::OUTPUT_PUSH_PULL);
  const uint64_t stop_time_ms{clock.current_time_millis() + DURATION_MS};

  gpio.write_pin(pin, 0);
  while (clock.current_time_millis() < stop_time_ms) {
    gpio.toggle_pin(pin);
    co_yield delay;
  }
  gpio.write_pin(pin, 0);
  co_return;
}

}  // namespace tvsc::hal::bringup
