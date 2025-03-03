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

class CountingClockable final : public Clockable<MockClock> {
 public:
  int update_time_call_count{};
  int run_call_count{};

  MockClock::time_point next_time{MockClock::time_point::max()};

  CountingClockable(MockClock& clock) : Clockable(clock) {}

  MockClock::time_point update_time(MockClock::time_point t) noexcept override {
    ++update_time_call_count;
    MockClock::time_point result{std::min(next_time, t)};
    next_time = MockClock::time_point::max();
    return result;
  }

  void run(MockClock::time_point t) noexcept override { ++run_call_count; }
};

TEST(MockClockTest, SettingTimeUpdatesClockables) {
  MockClock& clock{MockClock::clock()};
  CountingClockable counts{clock};
  EXPECT_EQ(0, counts.update_time_call_count);
  EXPECT_EQ(0, counts.run_call_count);
  clock.increment_current_time(42ms);
  EXPECT_EQ(1, counts.update_time_call_count);
  EXPECT_EQ(1, counts.run_call_count);
}

TEST(MockClockTest, ClockablesCanRollbackTimeUpdate) {
  MockClock& clock{MockClock::clock()};
  CountingClockable counts{clock};
  counts.next_time = MockClock::time_point{21ms};
  EXPECT_EQ(0, counts.update_time_call_count);
  EXPECT_EQ(0, counts.run_call_count);
  clock.increment_current_time(42ms);
  EXPECT_EQ(2, counts.update_time_call_count);
  EXPECT_EQ(2, counts.run_call_count);
}

}  // namespace tvsc::hal::time
