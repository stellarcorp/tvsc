#pragma once

#include <chrono>

#include "hal2/rcc/rcc.h"
#include "hal2/systick/systick.h"
#include "hal2/time_type.h"
#include "hal2/timer/timer.h"

namespace tvsc::hal2::clock {

enum class SleepStrategy {
  WAIT,
  SLEEP,
  STOP,
};

/**
 * Class to manage the compute the current time and handle sleep requests. This class is modelled
 * after the C++ Clock named requirement, but it does not include static accessors like the now()
 * method.
 */
template <SleepStrategy SLEEP_STRATEGY = SleepStrategy::SLEEP>
class Clock final {
 private:
  systick::SysTickType* systick_;
  rcc::Rcc* rcc_;
  // Low-power timer. Note that we are keeping this timer on the whole time, since we expect to use
  // it often.
  timer::Timer timer_;

 public:
  // C++ Clock types.
  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<Clock, duration>;

  // This clock always moves forward.
  static constexpr bool is_steady{true};

  Clock(systick::SysTickType& systick,  //
        rcc::Rcc& rcc,                  //
        timer::TimerPeripheral& timer_peripheral) noexcept
      : systick_(&systick), rcc_(&rcc), timer_(timer_peripheral.access()) {}

  [[nodiscard]] TimeType current_time_micros() const noexcept {
    return systick_->current_time_micros();
  }

  [[nodiscard]] TimeType current_time_millis() const noexcept {
    return systick_->current_time_micros() / 1000;
  }

  [[nodiscard]] time_point current_time() const noexcept {
    return time_point{std::chrono::microseconds{current_time_micros()}};
  }

  void sleep_ms(TimeType milliseconds) noexcept { sleep_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) noexcept {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void sleep(time_point t) noexcept { sleep(t - current_time()); }

  void wait_ms(TimeType milliseconds) noexcept { wait_us(milliseconds * 1000); }

  template <typename Rep, typename Period>
  void wait(std::chrono::duration<Rep, Period> d) noexcept {
    wait_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }

  void wait(time_point t) noexcept { wait(t - current_time()); }

  void wait_us(TimeType microseconds) noexcept {
    const auto wait_until_time_us{systick_->current_time_micros() + microseconds};
    while (systick_->current_time_micros() < wait_until_time_us) {
      // Do nothing. This is just a busy loop.
    }
  }

  void sleep_us(TimeType microseconds) noexcept
    requires(SLEEP_STRATEGY == SleepStrategy::STOP)
  {
    static constexpr TimeType TIME_TO_START_TIMER_US{25};
    static constexpr TimeType TIME_TO_WAKE_FROM_STOP_MODE_US{500};

    // We can't achieve any better precision than this, so just don't bother.
    if (microseconds < TIME_TO_START_TIMER_US) {
      return;
    }

    // Configure the timer. Assume that it will trigger an interrupt at the end of the interval.
    // Then enter stop mode. We exit stop mode on any interrupt (or possibly any EXTI event as
    // well). So, we wrap the call to enter sleep/stop mode with a check to see if the timer is
    // still running; when it is not running, its interrupt has fired. Alternatively, for short
    // sleeps where the sleep time is less than the time it takes to wake from stop mode, we simply
    // block on WFI (Wait For Interrupt). Note: entering stop/sleep mode is only 1-2 clock cycles,
    // so we ignore that time.
    timer_.configure_single_pulse(microseconds);
    systick_->mask_interrupt();
    if (microseconds < TIME_TO_WAKE_FROM_STOP_MODE_US) {
      timer_.start();
      do {
        rcc_->enter_sleep_mode();
      } while (timer_.is_running());
    } else {
      timer_.start();
      do {
        // TODO(james): Implement system bus WAKE line. CAN bus can't wake the MCU from stop mode,
        // and CAN transceivers that are able to approximate that capability requiring transmitting
        // an extra "wakeup" packet for each real packet. As CAN bus transmissions are orders of
        // magnitude more energy intensive than almost anything else, we need a different technique
        // to avoid those extra transmissions.
        rcc_->enter_stop_mode();
      } while (timer_.is_running());
    }
    systick_->unmask_interrupt();
    systick_->increment_micros(microseconds);
  }

  void sleep_us(TimeType microseconds) noexcept
    requires(SLEEP_STRATEGY == SleepStrategy::SLEEP)
  {
    static constexpr TimeType TIME_TO_START_TIMER_US{25};

    // We can't achieve any better precision than this, so just don't bother.
    if (microseconds < TIME_TO_START_TIMER_US) {
      return;
    }

    // Configure the timer. Assume that it will trigger an interrupt at the end of the interval.
    // Then enter stop mode. We exit stop mode on any interrupt (or possibly any EXTI event as
    // well). So, we wrap the call to enter sleep mode with a check to see if the timer is still
    // running; when it is not running, its interrupt has fired.
    timer_.configure_single_pulse(microseconds);
    systick_->mask_interrupt();
    timer_.start();
    do {
      rcc_->enter_sleep_mode();
    } while (timer_.is_running());
    systick_->unmask_interrupt();
    systick_->increment_micros(microseconds);
  }

  void sleep_us(TimeType microseconds) noexcept
    requires(SLEEP_STRATEGY == SleepStrategy::WAIT)
  {
    wait_us(microseconds);
  }
};

}  // namespace tvsc::hal2::clock
