#include "hal/watchdog/stm32l4xx_watchdog.h"

#include <chrono>
#include <cstdlib>

#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::watchdog {

void WatchdogStm32l4xx::feed() { HAL_IWDG_Refresh(&watchdog_); }

std::chrono::milliseconds WatchdogStm32l4xx::reset_interval() {
  using namespace std::chrono_literals;
  return 1000ms;
}

void WatchdogStm32l4xx::enable() {
  lsi_active_ = oscillator_->access();

  watchdog_.Init.Prescaler = IWDG_PRESCALER_32;  // LSI / 32 = 1 kHz
  watchdog_.Init.Reload = reset_interval().count();
  watchdog_.Init.Window = watchdog_.Init.Reload;
  HAL_IWDG_Init(&watchdog_);
}

void WatchdogStm32l4xx::disable() {
  // Watchdogs can't be disabled by design, except on system reset.
  // Attempting to disable the watchdog means that we are no longer actively feeding it. In that
  // case, it is best to fail sooner, rather than wait for the watchdog to reset the system.
  abort();
}

}  // namespace tvsc::hal::watchdog
