#include "hal/watchdog/stm32l4xx_watchdog.h"

#include <chrono>
#include <cstdlib>

#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

__attribute__((section(".status.value"))) volatile uint32_t watchdog_reload_value{};

namespace tvsc::hal::watchdog {

void WatchdogStm32l4xx::feed() { HAL_IWDG_Refresh(&watchdog_); }

std::chrono::milliseconds WatchdogStm32l4xx::reset_interval() {
  using namespace std::chrono_literals;
  return 2s;
}

void WatchdogStm32l4xx::enable() {
  lsi_active_ = oscillator_->access();

  watchdog_.Init.Prescaler = IWDG_PRESCALER_32;  // LSI / 32 = 1 kHz
  // Ensure that the reset_interval() is expressed in milliseconds before we call the count()
  // method. The prescaler above puts the IWDG counting in milliseconds. If the reset_interval is in
  // a different unit, we will have a mismatch.
  const auto interval{std::chrono::duration_cast<std::chrono::milliseconds>(reset_interval())};
  watchdog_reload_value = interval.count();
  // error(watchdog_reload_value <= 0x0fff);
  watchdog_.Init.Reload = watchdog_reload_value;
  watchdog_.Init.Window = 0x0fff;
  HAL_IWDG_Init(&watchdog_);
}

void WatchdogStm32l4xx::disable() {
  // Watchdogs can't be disabled by design, except on system reset.
  // Attempting to disable the watchdog means that we are no longer actively feeding it. In that
  // case, it is best to fail sooner, rather than wait for the watchdog to reset the system.
  // error();
}

}  // namespace tvsc::hal::watchdog
