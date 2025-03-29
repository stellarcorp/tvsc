#include <chrono>
#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/time_type.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"

namespace tvsc::bringup {

using namespace std::chrono_literals;

template <typename ClockType,
          uint64_t DURATION_MS = 365UL * 24 * 60 * 60 * 1000 /* one year in milliseconds */>
tvsc::scheduler::Task<ClockType> blink(ClockType& clock,
                                       tvsc::hal::gpio::GpioPeripheral& gpio_peripheral,
                                       tvsc::hal::gpio::Pin pin,
                                       typename ClockType::duration delay = 500ms) {
  tvsc::hal::gpio::Gpio gpio{gpio_peripheral.access()};

  gpio.set_pin_mode(pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
  const uint64_t stop_time_ms{clock.current_time_millis() + DURATION_MS};

  gpio.write_pin(pin, 0);
  while (clock.current_time_millis() < stop_time_ms) {
    gpio.toggle_pin(pin);
    co_yield delay;
  }
  gpio.write_pin(pin, 0);
  co_return;
}

}  // namespace tvsc::bringup
