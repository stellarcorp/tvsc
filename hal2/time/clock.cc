#include "hal2/time/clock.h"

#include <cstdint>

#include "hal2/time_type.h"

namespace tvsc::hal2::time {

void Clock::sleep_us(TimeType microseconds) noexcept {
  static constexpr TimeType TIME_TO_START_TIMER_US{25};
  static constexpr TimeType TIME_TO_WAKE_FROM_STOP_MODE_US{500};

  // We can't achieve any better precision than this, so just don't bother.
  if (microseconds < TIME_TO_START_TIMER_US) {
    return;
  }

  // Configure the timer. Assume that it will trigger an interrupt at the end of the interval. Then
  // enter stop mode. We exit stop mode on any interrupt (or possibly any EXTI event as well). So,
  // we wrap the call to enter stop mode with a check to see if the timer is still running; when it
  // is not running, its interrupt has fired. Alternatively, for short sleeps where the sleep time
  // is less than the time it takes to wake from stop mode, we simply block on WFI (Wait For
  // Interrupt). Note: entering stop/sleep mode is only 1-2 clock cycles, so we ignore that time.
  timer_.configure_single_pulse(microseconds);
  if (microseconds < TIME_TO_WAKE_FROM_STOP_MODE_US) {
    systick_->mask_interrupt();
    timer_.start();
    do {
      rcc_->enter_sleep_mode();
    } while (timer_.is_running());
    systick_->unmask_interrupt();
  } else {
    systick_->mask_interrupt();
    timer_.start();
    do {
      // TODO(james): Issue 23. Fix the CAN bus so that it can wake from stop 1 mode when receiving
      // a message. Then, remove the next line and replace it with
      // rcc_->enter_stop_mode();
      // This change would put the MCU in stop mode during a long sleep, rather than just sleep
      // mode, and should result in substantial power savings.
      // TODO(james): Implement system bus WAKE line. CAN bus can't wake the MCU from stop mode, and
      // CAN transceivers that are able to approximate that capability requiring transmitting an
      // extra "wakeup" packet for each real packet. As CAN bus transmissions are orders of
      // magnitude more energy intensive than almost anything else, we need a different technique to
      // avoid those extra transmissions.
      rcc_->enter_sleep_mode();
    } while (timer_.is_running());
    systick_->unmask_interrupt();
  }

  systick_->increment_micros(microseconds);
}

void Clock::wait_us(TimeType microseconds) noexcept {
  const auto wait_until_time_us{systick_->current_time_micros() + microseconds};
  while (systick_->current_time_micros() < wait_until_time_us) {
    // Do nothing. This is just a busy loop.
  }
}

}  // namespace tvsc::hal2::time
