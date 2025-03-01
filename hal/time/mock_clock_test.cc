#include "hal/time/mock_clock.h"

#include "gtest/gtest.h"

namespace tvsc::hal::time {

TEST(MockClockTest, ReturnsSetTime) {
  MockClock& clock{MockClock::clock()};
  clock.set_current_time_millis(42);
  EXPECT_EQ(42, clock.current_time_millis());
}

TEST(MockClockTest, SleepMsUpdatesCurrentTime) {
  MockClock& clock{MockClock::clock()};
  clock.set_current_time_millis(42);
  ASSERT_EQ(42, clock.current_time_millis());
  clock.sleep_ms(8);
  EXPECT_EQ(50, clock.current_time_millis());
}

TEST(MockClockTest, SleepUsUpdatesCurrentTime) {
  MockClock& clock{MockClock::clock()};
  clock.set_current_time_millis(42);
  ASSERT_EQ(42, clock.current_time_millis());
  clock.sleep_us(8000);
  EXPECT_EQ(50, clock.current_time_millis());
}

TEST(MockClockTest, SleepUsUpdatesCurrentTimeWithRoundOff) {
  MockClock& clock{MockClock::clock()};
  clock.set_current_time_millis(42);
  ASSERT_EQ(42, clock.current_time_millis());
  clock.sleep_us(8124);
  EXPECT_EQ(50, clock.current_time_millis());
}

}  // namespace tvsc::hal::time
