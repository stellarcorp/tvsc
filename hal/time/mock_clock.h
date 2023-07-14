#include <cstdint>

#include "hal/time/clock.h"

namespace tvsc::hal::time {

class MockClock final : public Clock {
 private:
  uint64_t current_time_ms_{};

 public:
  uint64_t current_time_millis() override;
  void set_current_time_millis(uint64_t current_time_ms);
};

}  // namespace tvsc::hal::time
