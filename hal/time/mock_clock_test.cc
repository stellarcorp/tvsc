#include "hal/time/mock_clock.h"

#include "gtest/gtest.h"

extern "C" {

volatile uint64_t uwTick{};

}  // extern "C"

namespace tvsc::hal::time {

TEST(MockClockTest, ReturnsSetTime) {
  tvsc::hal::time::MockClock clock{};
  clock.set_current_time_millis(42);
  EXPECT_EQ(42, clock.current_time_millis());
}

TEST(MockClockTest, SleepMsUpdatesCurrentTime) {
  tvsc::hal::time::MockClock clock{};
  clock.set_current_time_millis(42);
  ASSERT_EQ(42, clock.current_time_millis());
  clock.sleep_ms(8);
  EXPECT_EQ(50, clock.current_time_millis());
}

TEST(MockClockTest, SleepUsUpdatesCurrentTime) {
  tvsc::hal::time::MockClock clock{};
  clock.set_current_time_millis(42);
  ASSERT_EQ(42, clock.current_time_millis());
  clock.sleep_us(8000);
  EXPECT_EQ(50, clock.current_time_millis());
}

TEST(MockClockTest, SleepUsUpdatesCurrentTimeWithRoundOff) {
  tvsc::hal::time::MockClock clock{};
  clock.set_current_time_millis(42);
  ASSERT_EQ(42, clock.current_time_millis());
  clock.sleep_us(8124);
  EXPECT_EQ(50, clock.current_time_millis());
}

}  // namespace tvsc::hal::time
