#pragma once

#include <cstdint>
#include <vector>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

class Clockable;

class MockClock final : public Clock {
 private:
  uint64_t current_time_us_{};

  std::vector<Clockable*> clockables_{};

  void update_clockables();

 public:
  void register_clockable(Clockable& clockable) { clockables_.push_back(&clockable); }

  uint64_t current_time_millis() override;
  uint64_t current_time_micros() override;

  void sleep_ms(uint32_t milliseconds) override;
  void sleep_us(uint32_t microseconds) override;

  // Setters/modifiers for testing.
  void set_current_time_millis(uint64_t current_time_ms);
  void increment_current_time_millis(uint64_t increment_ms = 1);

  void set_current_time_micros(uint64_t current_time_us);
  void increment_current_time_micros(uint64_t increment_us = 1);
};

/**
 * A Clockable is a test-only mock that represents a parallel processing subsystem, such as a
 * daughterboard or external chipset. A Clockable registers itself with the MockClock, and then any
 * subsequent calls to set the time result in a call to update the Clockable. The Clockable should
 * then modify its state to reflect the new time.
 */
class Clockable {
 protected:
  MockClock* clock_;

 public:
  Clockable(MockClock& clock) : clock_(&clock) { clock_->register_clockable(*this); }
  virtual ~Clockable() = default;

  virtual void update(uint64_t current_time_us) = 0;
};

}  // namespace tvsc::hal::time
