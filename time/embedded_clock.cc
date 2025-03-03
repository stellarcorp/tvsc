#include "time/embedded_clock.h"

#include <cstdint>

#include "hal/board/board.h"
#include "time/clock.h"

namespace tvsc::time {

TimeType EmbeddedClock::current_time_micros() noexcept { return sys_tick_->current_time_micros(); }
TimeType EmbeddedClock::current_time_millis() noexcept {
  return sys_tick_->current_time_micros() / 1000;
}

void EmbeddedClock::sleep_us(TimeType microseconds) noexcept {
  static constexpr TimeType TIME_TO_START_TIMER_US{25};
  static constexpr TimeType TIME_TO_WAKE_FROM_STOP_MODE_US{500};

  // We can't achieve any better precision than this, so just don't bother.
  if (microseconds < TIME_TO_START_TIMER_US) {
    return;
  }

  // Start the timer. Assume that it will trigger an interrupt at the end of the interval. Then
  // enter stop mode. We exit stop mode on any interrupt (or possibly any EXTI event as well). So,
  // we wrap the call to enter stop mode with a check to see if the timer is still running; when it
  // is not running, its interrupt has fired. Alternatively, for short sleeps where the sleep time
  // is less than the time it takes to wake from stop mode, we simply block on WFI (Wait For
  // Interrupt). Note: entering stop mode is only 1-2 clock cycles, so we ignore that time.
  timer_.start(microseconds);
  if (microseconds < TIME_TO_WAKE_FROM_STOP_MODE_US) {
    while (timer_.is_running()) {
      power_peripheral_->enter_sleep_mode();
    }
  } else {
    while (timer_.is_running()) {
      power_peripheral_->enter_stop_mode();
    }
    // In stop mode, the SysTick is not running, so we manually update the tick counter with the
    // amount of time we spent in stop mode.
    sys_tick_->increment_micros(microseconds);
    rcc_->restore_clock_speed();
  }
}

void EmbeddedClock::sleep_ms(TimeType milliseconds) noexcept { sleep_us(milliseconds * 1000); }

EmbeddedClock::time_point EmbeddedClock::now() noexcept { return clock().current_time(); }

EmbeddedClock& EmbeddedClock::clock() noexcept {
  static EmbeddedClock instance{
      tvsc::hal::board::Board::board().sys_tick(),     //
      tvsc::hal::board::Board::board().sleep_timer(),  //
      tvsc::hal::board::Board::board().power(),        //
      tvsc::hal::board::Board::board().rcc()           //
  };
  return instance;
}

}  // namespace tvsc::time
