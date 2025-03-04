#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "bringup/blink.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

extern "C" {

__attribute__((section(".status.value"))) volatile uint32_t watchdog_counter{};

}  // extern "C"

namespace tvsc::bringup {

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;
using TaskType = tvsc::scheduler::Task<ClockType>;

using namespace std::chrono_literals;

template <typename Duration = std::chrono::days>
TaskType run_watchdog(ClockType& clock,
                      tvsc::hal::watchdog::WatchdogPeripheral& watchdog_peripheral,
                      Duration reset_in = std::chrono::duration_cast<Duration>(24h)) {
  const auto feed_interval{watchdog_peripheral.reset_interval() / 4};
  const auto reset_at{clock.current_time() + reset_in};

  // Enable the watchdog here. After this, if the dog isn't fed on time, the board will reset.
  auto watchdog{watchdog_peripheral.access()};
  while (clock.current_time() < reset_at) {
    co_yield feed_interval;
    watchdog.feed();
    ++watchdog_counter;
  }

  // Stop feeding, but keep the watchdog instance alive. This will trigger a reset.
  while (true) {
    co_yield feed_interval;
    ++watchdog_counter;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main() {
  static constexpr auto CYCLE_TIME{5s};
  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  // Sleep now so that we can detect the board reset. If we just launch into the tasks, we might not
  // be able to detect that the watchdog caused a reset.
  clock.sleep(CYCLE_TIME);

  Scheduler<ClockType, 4 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(run_watchdog(clock, board.iwdg(), CYCLE_TIME));
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::GREEN_LED_PORT>(), BoardType::GREEN_LED_PIN));
  scheduler.start();
}
