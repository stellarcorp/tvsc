#include "hal/time/mock_clock.h"

#include <chrono>

#include "gtest/gtest.h"

namespace tvsc::hal::time {

using namespace std::chrono_literals;

TEST(MockClockTest, ReturnsSetTime) {
  MockClock& clock{MockClock::clock()};
  const auto initial_time{clock.current_time()};
  clock.increment_current_time(42ms);
  EXPECT_EQ(initial_time + 42ms, clock.current_time());
}

TEST(MockClockTest, SleepDurationUpdatesCurrentTime) {
  MockClock& clock{MockClock::clock()};
  const auto initial_time{clock.current_time()};
  clock.increment_current_time(42ms);
  ASSERT_EQ(initial_time + 42ms, clock.current_time());
  clock.sleep(8ms);
  EXPECT_EQ(initial_time + 50ms, clock.current_time());
}

TEST(MockClockTest, SleepTimePointUpdatesCurrentTime) {
  MockClock& clock{MockClock::clock()};
  const auto initial_time{clock.current_time()};
  clock.increment_current_time(42ms);
  ASSERT_EQ(initial_time + 42ms, clock.current_time());
  clock.sleep(initial_time + 50ms);
  EXPECT_EQ(initial_time + 50ms, clock.current_time());
}

}  // namespace tvsc::hal::time
