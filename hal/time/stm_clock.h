#pragma once

#include <chrono>
#include <ratio>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

/**
 * Class to manage the compute the current time and handle sleep requests for the STM32H7xx.
 */
class ClockStm32xxxx final : public time::Clock {
 public:
  TimeType current_time_micros() override;
  TimeType current_time_millis() override;

  void sleep_us(TimeType microseconds) override;
  void sleep_ms(TimeType milliseconds) override;

  // C++ Clock requirements:
  // The number of seconds per tick. We use STM's HAL that fixes the frequency of the ticks at one
  // tick every millisecond. See HAL_InitTick() for the details.
  using rep = TimeType;  // TODO(james): Flip these around. Remove TimeType and make this the
                         // canonical type.
  using period = std::milli;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<ClockStm32xxxx, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

  static time_point now() noexcept;
};

}  // namespace tvsc::hal::time
