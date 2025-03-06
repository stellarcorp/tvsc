#include "hal/simulation/reactor.h"

#include <chrono>

#include "gtest/gtest.h"
#include "time/mock_clock.h"

namespace tvsc::hal::simulation {

using ClockType = time::MockClock;
using namespace std::chrono_literals;

TEST(ReactorTest, CanInstantiate) { Reactor r{ClockType::clock()}; }

class TestEventGenerator final : public EventGenerator<ClockType> {
 public:
  int times_called{};

  ClockType::duration next_event_in(ClockType::time_point now) const noexcept override {
    return 10ms;
  }

  void generate(ClockType::time_point now) noexcept override { ++times_called; }
};

TEST(ReactorTest, CanRunGeneratorOnce) {
  TestEventGenerator generator{};
  {
    ClockType& clock{ClockType::clock()};
    Reactor r{clock};
    r.add_generator(generator);
    EXPECT_EQ(0, generator.times_called);
    clock.increment_current_time(15ms);
  }
  EXPECT_EQ(1, generator.times_called);
}

TEST(ReactorTest, CanRunGeneratorOnInterval) {
  TestEventGenerator generator{};
  {
    ClockType& clock{ClockType::clock()};
    Reactor r{clock};
    r.add_generator(generator);
    EXPECT_EQ(0, generator.times_called);
    clock.increment_current_time(10ms);
    clock.increment_current_time(10ms);
    clock.increment_current_time(20ms);
  }
  EXPECT_EQ(4, generator.times_called);
}

}  // namespace tvsc::hal::simulation
