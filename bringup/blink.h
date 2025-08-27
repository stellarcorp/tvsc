#include <chrono>
#include <cstring>
#include <limits>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "hal/time_type.h"
#include "system/system.h"
#include "system/task.h"

namespace tvsc::bringup {

using namespace std::chrono_literals;

template <tvsc::hal::TimeType DURATION_MS =
              /* one year in milliseconds */ 365LL * 24 * 60 * 60 * 1000>
tvsc::system::System::Task blink(tvsc::hal::gpio::PinPeripheral led_peripheral,
                                 typename system::System::ClockType::duration delay = 500ms) {
  tvsc::hal::gpio::Pin led{led_peripheral.access()};

  led.set_pin_mode(tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
  const auto stop_time{system::System::clock().current_time() +
                       std::chrono::milliseconds(DURATION_MS)};

  led.write_pin(0);
  while (system::System::clock().current_time() < stop_time) {
    led.toggle_pin();
    co_yield delay;
  }
  led.write_pin(0);
  co_return;
}

tvsc::system::System::Task blink_on_success(std::function<bool()> is_success,
                                            tvsc::hal::gpio::GpioPeripheral& led_peripheral,
                                            tvsc::hal::gpio::PinNumber led_pin) {
  tvsc::hal::gpio::Gpio led_gpio{led_peripheral.access()};

  led_gpio.set_pin_mode(led_pin, tvsc::hal::gpio::PinMode::OUTPUT_PUSH_PULL);
  led_gpio.write_pin(led_pin, 0);
  constexpr typename system::System::ClockType::duration success_delay{250ms};
  constexpr typename system::System::ClockType::duration fail_delay1{50ms};
  constexpr typename system::System::ClockType::duration fail_delay2{500ms};
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
