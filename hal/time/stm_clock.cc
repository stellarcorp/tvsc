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
  // Start the timer. Assume that it will trigger an interrupt at the end of the interval. Then
  // enter stop mode. We exit stop mode on any interrupt (or possibly any EXTI event as well). So,
  // we wrap the call to enter stop mode with a check to see if the timer is still running.
  timer_.start(microseconds);
  if (microseconds < 500) {
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
