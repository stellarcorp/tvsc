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

  // C++ Clock requirements.
  // We do not implement the static now() function. This means that our Clock does not satisfy
  // std::chrono::is_clock. Implementing that function causes a circular dependency between the
  // //hal/time target and the //hal/board target. To implement it correctly, we want to use the
  // Clock instance from the board in the definition of now(). That's a dependency on //hal/board
  // from //hal/time. But we want to instantiate the specific Clock instance in the Board. That's a
  // dependency on //hal/time from //hal/board. We can break this dependency fairly easily, at the
  // expense of creating extra implementation targets in the //hal/time package, but then, every
  // binary would need to add an extra dependency. Ultimately, we use current_time() instead of the
  // static now(), so dropping it from the API is the easiest approach, even at the expense of
  // losing std::chrono::is_clock compatibility.

  using rep = TimeType;
  using period = std::micro;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<Clock, duration>;

  // This clock always moves forward and is never adjusted.
  static constexpr bool is_steady{true};

  virtual TimeType current_time_millis() = 0;
  virtual TimeType current_time_micros() = 0;

  time_point current_time() noexcept {
    return time_point{std::chrono::microseconds{current_time_micros()}};
  }

  virtual void sleep_ms(TimeType milliseconds) = 0;
  virtual void sleep_us(TimeType microseconds) = 0;

  void sleep(time_point t) {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(t - current_time()).count());
  }

  template <typename Rep, typename Period>
  void sleep(std::chrono::duration<Rep, Period> d) {
    sleep_us(std::chrono::duration_cast<std::chrono::microseconds>(d).count());
  }
};

}  // namespace tvsc::hal::time
