#include <cstdint>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

class MockClock final : public Clock {
 private:
  uint64_t current_time_us_{};

 public:
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

}  // namespace tvsc::hal::time
