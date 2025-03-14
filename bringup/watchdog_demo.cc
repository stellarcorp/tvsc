#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/quit.h"
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
                      tvsc::hal::watchdog::WatchdogPeripheral& watchdog_peripheral) {
  const auto feed_interval{watchdog_peripheral.reset_interval() / 4};

  // Enable the watchdog here. After this, if the dog isn't fed on time, the board will reset.
  auto watchdog{watchdog_peripheral.access()};
  while (true) {
    co_yield feed_interval;
    watchdog.feed();
    ++watchdog_counter;
  }
}

}  // namespace tvsc::bringup

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  static constexpr auto CYCLE_TIME{5s};
  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  // Sleep now so that we can detect the board reset. If we just launch into the tasks, we might not
  // be able to detect that the watchdog caused a reset.
  clock.sleep(CYCLE_TIME);

  Scheduler<ClockType, 4 /*QUEUE_SIZE*/> scheduler{board.rcc()};
  scheduler.add_task(run_watchdog(clock, board.iwdg()));
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::GREEN_LED_PORT>(), BoardType::GREEN_LED_PIN));
  scheduler.add_task(quit(scheduler, CYCLE_TIME));
  scheduler.start();
}
