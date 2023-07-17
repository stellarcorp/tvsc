#include "hal/time/mock_clock.h"

#include "gtest/gtest.h"

namespace tvsc::hal::time {

TEST(MockClockTest, ReturnsSetTime) {
  tvsc::hal::time::MockClock clock{};
  clock.set_current_time_millis(42);
  EXPECT_EQ(42, clock.current_time_millis());
}

}  // namespace tvsc::hal::time
