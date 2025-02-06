#include "hal/time/stm_clock.h"

#include <chrono>
#include <concepts>

#include "gtest/gtest.h"

// Add tick definition so that we can compile. Note that this symbol is unused in this test.
volatile uint64_t uwTick{};

namespace tvsc::hal::time {

TEST(StmClockTest, SatisfiesClock) {
  static constexpr bool is_satisfied{std::chrono::is_clock_v<ClockStm32xxxx>};
  EXPECT_TRUE(is_satisfied);
}

}  // namespace tvsc::hal::time
