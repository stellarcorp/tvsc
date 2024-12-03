#pragma once

#include <cstdint>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

/**
 * Clock implementation that models the time on a remote clock, likely on another computer. This
 * Clock gives the time as it would be given by the remote clock, to the best of this Clock's
 * ability.
 *
 * Currently, this Clock uses a simple offset from the local clock to represent the time of the
 * remote clock. This is sufficient for some rudimentary purposes, but will need to be updated to
 * handle more complex models. In particular, this class needs to be capable of handling offsets
 * (which it currently does); differences in crystal frequencies, such as those caused by
 * temperature fluctuations; and relativistic effects.
 */
class RemoteClock final : public Clock {
 private:
  Clock* local_clock_;

  // Offset of remote clock wrt to the local clock. A negative value means that the remote clock is
  // "behind" and will return a smaller value for the current time than the local clock.
  // Example:
  // At 100us after the epoch according the local clock, with a skew of -25us, the remote clock
  // would indicate that it is 75us after the epoch. With a skew of (positive) 50us, the remote
  // clock would indicate that it is 150us after the epoch.
  // Note that this approach to representing remote time is very basic. Much more accurate methods
  // exist; see the details of NTP for better algorithms.
  double skew_us_{0.};

 public:
  RemoteClock(Clock& local_clock) : local_clock_(&local_clock) {}

  /**
   * Mark the time as it would currently be measured by the remote clock.
   */
  void mark_remote_time_micros(TimeType remote_time_micros);

  TimeType current_time_millis() override;
  TimeType current_time_micros() override;

  void sleep_ms(TimeType milliseconds) override;
  void sleep_us(TimeType microseconds) override;
};

}  // namespace tvsc::hal::time
