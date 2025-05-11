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

template <typename ClockType>
tvsc::scheduler::Task<ClockType> blink_on_success(std::function<bool()> is_success,
                                                  tvsc::hal::gpio::GpioPeripheral& led_peripheral,
                                                  tvsc::hal::gpio::Pin led_pin) {
  tvsc::hal::gpio::Gpio led_gpio{led_peripheral.access()};

  led_gpio.set_pin_mode(led_pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
  led_gpio.write_pin(led_pin, 0);
  constexpr typename ClockType::duration success_delay{250ms};
  constexpr typename ClockType::duration fail_delay1{50ms};
  constexpr typename ClockType::duration fail_delay2{500ms};
  constexpr int fail_toggle_target{6};

  bool success{is_success()};
  int fail_toggle_count{0};

  while (true) {
    while (success) {
      led_gpio.toggle_pin(led_pin);
      co_yield success_delay;
      success = is_success();
    }

    while (!success) {
      fail_toggle_count = 0;
      while (!success && fail_toggle_count < fail_toggle_target) {
        led_gpio.toggle_pin(led_pin);
        co_yield fail_delay1;
        ++fail_toggle_count;
        success = is_success();
      }
      if (!success) {
        led_gpio.toggle_pin(led_pin);
        co_yield fail_delay2;
        success = is_success();
      }
    }
  }
}

}  // namespace tvsc::bringup
