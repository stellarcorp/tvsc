#include "hal/time/stm_clock.h"

#include <cstdint>

#include "hal/time/clock.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

extern "C" {

extern volatile uint64_t uwTick;

void SysTick_Handler() { ++uwTick; }

}  // extern "C"

namespace tvsc::hal::time {

TimeType ClockStm32xxxx::current_time_micros() { return uwTick * 1000; }
TimeType ClockStm32xxxx::current_time_millis() { return uwTick; }

void ClockStm32xxxx::sleep_us(TimeType microseconds) {
  static constexpr TimeType TIME_TO_WAKE_FROM_STOP_MODE_US{500};
  // Start the timer. Assume that it will trigger an interrupt at the end of the interval. Then
  // enter stop mode. We exit stop mode on any interrupt (or possibly any EXTI event as well). So,
  // we wrap the call to enter stop mode with a check to see if the timer is still running; when it
  // is not running, its interrupt has fired. Alternatively, for short sleeps where the sleep time
  // is less than the time it takes to wake from stop mode, we simply block on WFI (Wait For
  // Interrupt). Note: entering stop mode is only 1-2 clock cycles, so we ignore that time.
  timer_.start(microseconds);
  if (microseconds < TIME_TO_WAKE_FROM_STOP_MODE_US) {
    while (timer_.is_running()) {
      __WFI();
    }
  } else {
    while (timer_.is_running()) {
      power_peripheral_->enter_stop_mode();
    }
    uwTick += microseconds / 1000;
    rcc_->restore_clock_speed();
  }
}

void ClockStm32xxxx::sleep_ms(TimeType milliseconds) { sleep_us(milliseconds * 1000); }

}  // namespace tvsc::hal::time
