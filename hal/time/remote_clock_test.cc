#include "hal/time/remote_clock.h"

#include "gtest/gtest.h"
#include "hal/time/mock_clock.h"

namespace tvsc::hal::time {

TEST(RemoteClockTest, CanCompile) {
  MockClock local_clock{};
  RemoteClock remote_clock{local_clock};
  EXPECT_EQ(local_clock.current_time_micros(), remote_clock.current_time_micros());
}

TEST(RemoteClockTest, TracksLocalClockIfNoMarkedTimes) {
  MockClock local_clock{};
  RemoteClock remote_clock{local_clock};
  EXPECT_EQ(local_clock.current_time_micros(), remote_clock.current_time_micros());

  local_clock.set_current_time_micros(100);

  EXPECT_EQ(local_clock.current_time_micros(), remote_clock.current_time_micros());
}

TEST(RemoteClockTest, MarkingBehindTimeCreatesNegativeOffset) {
  MockClock local_clock{};
  RemoteClock remote_clock{local_clock};

  constexpr uint64_t LOCAL_TIME_US{100};
  constexpr int64_t OFFSET_US{-42};

  ASSERT_EQ(local_clock.current_time_micros(), remote_clock.current_time_micros());

  local_clock.set_current_time_micros(LOCAL_TIME_US);
  remote_clock.mark_remote_time_micros(LOCAL_TIME_US + OFFSET_US);

  EXPECT_EQ(local_clock.current_time_micros() + OFFSET_US, remote_clock.current_time_micros());
}

TEST(RemoteClockTest, MarkingAheadTimeCreatesPositiveOffset) {
  MockClock local_clock{};
  RemoteClock remote_clock{local_clock};

  constexpr uint64_t LOCAL_TIME_US{100};
  constexpr int64_t OFFSET_US{42};

  ASSERT_EQ(local_clock.current_time_micros(), remote_clock.current_time_micros());

  local_clock.set_current_time_micros(LOCAL_TIME_US);
  remote_clock.mark_remote_time_micros(LOCAL_TIME_US + OFFSET_US);

  EXPECT_EQ(local_clock.current_time_micros() + OFFSET_US, remote_clock.current_time_micros());
}

TEST(RemoteClockTest, TracksLocalClockIfHaveMarkedTimes) {
  MockClock local_clock{};
  RemoteClock remote_clock{local_clock};

  constexpr uint64_t FIRST_LOCAL_TIME_US{100};
  constexpr int64_t OFFSET_US{-42};

  local_clock.set_current_time_micros(FIRST_LOCAL_TIME_US);
  remote_clock.mark_remote_time_micros(FIRST_LOCAL_TIME_US + OFFSET_US);

  EXPECT_EQ(local_clock.current_time_micros() + OFFSET_US, remote_clock.current_time_micros());

  constexpr uint64_t SECOND_LOCAL_TIME_US{100};
  local_clock.set_current_time_micros(SECOND_LOCAL_TIME_US);
  EXPECT_EQ(local_clock.current_time_micros() + OFFSET_US, remote_clock.current_time_micros());
}

}  // namespace tvsc::hal::time
