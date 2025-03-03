#pragma once

namespace tvsc::time {

/**
 * A Clockable is a test- and simulation-only mock that represents a parallel processing subsystem,
 * such as a daughterboard or external chipset. A Clockable registers itself with the MockClock, and
 * then any subsequent calls to set the time result in a call to update the Clockable. The Clockable
 * should then modify its state to reflect the new time.
 */
template <typename ClockT>
class Clockable {
 public:
  using ClockType = ClockT;

 protected:
  ClockType* clock_;

 public:
  Clockable(ClockType& clock) noexcept : clock_(&clock) { clock_->register_clockable(*this); }
  virtual ~Clockable() noexcept { clock_->deregister_clockable(*this); }

  [[nodiscard]] virtual ClockType::time_point update_time(
      ClockType::time_point current_time) noexcept = 0;

  virtual void run(ClockType::time_point current_time) noexcept = 0;
};

}  // namespace tvsc::time
