#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>

extern "C" {
typedef uint64_t CTimeType;
}

namespace tvsc::hal::time {

// Type for representing times (usually a count of seconds, milliseconds, or microseconds) as
// scalars.
using TimeType = uint64_t;

class Clock {
 public:
  virtual ~Clock() = default;

  virtual TimeType current_time_millis() = 0;
  virtual TimeType current_time_micros() = 0;

  virtual void sleep_ms(TimeType milliseconds) = 0;
  virtual void sleep_us(TimeType microseconds) = 0;

  // C++ Clock requirements.
  // We assume that the implementations of this interface will be implement the system_clock()
  // function below. That function is used in the now() function in this interface. We could have
  // also used the curiously recurring template pattern, but this is the current structure.

  // TODO(james): Flip these around. Remove TimeType and make this the canonical type.
  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<Clock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

  static time_point now() noexcept;
};

/**
 * Accessor to a global instance of a steady clock that acts as the primary clock for the system.
 * Unlike C++'s system_clock type, this clock should be steady. Note that the result of this
 * function is used in the now() function above.
 */
Clock& system_clock();

}  // namespace tvsc::hal::time
