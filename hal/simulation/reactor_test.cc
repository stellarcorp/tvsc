#include "hal/simulation/reactor.h"

#include <chrono>

#include "gtest/gtest.h"
#include "hal/simulation/irq_generator.h"
#include "time/mock_clock.h"

namespace tvsc::hal::simulation {

using ClockType = time::MockClock;
using namespace std::chrono_literals;

TEST(ReactorTest, CanInstantiate) { Reactor r{ClockType::clock()}; }

class TestIrqGenerator final : public IrqGenerator<ClockType> {
 public:
  int times_called{};

  int irq() const noexcept override { return 42; }

  const char* irq_name() const noexcept override { return "test interrupt"; }

  ClockType::duration next_interrupt_in(ClockType::time_point now) const noexcept override {
    return 10ms;
  }

  void handle_interrupt() noexcept override { ++times_called; }
};

TEST(ReactorTest, CanRunGeneratorOnce) {
  TestIrqGenerator generator{};
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
  TestIrqGenerator generator{};
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
