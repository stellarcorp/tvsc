#include "bringup/monitor_power.h"

#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bringup/quit.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;

extern "C" {

__attribute__((section(".status.value"))) PowerUsage power_monitor1{};
__attribute__((section(".status.value"))) PowerUsage power_monitor2{};

}  // extern "C"

static constexpr size_t QUEUE_SIZE{3};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(monitor_power<ClockType>(board.power_monitor1(), power_monitor1));
  scheduler.add_task(monitor_power<ClockType>(board.power_monitor2(), power_monitor2));
  scheduler.add_task(quit(scheduler));

  scheduler.start();
}
