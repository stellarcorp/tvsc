#include "bringup/monitor_power.h"

#include <chrono>
#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "hal/board/board.h"
#include "scheduler/scheduler.h"
#include "scheduler/task.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::scheduler;
using namespace std::chrono_literals;

extern "C" {

__attribute__((section(".status.value"))) PowerUsage power_monitor1{};
__attribute__((section(".status.value"))) PowerUsage power_monitor2{};

}  // extern "C"

static constexpr size_t QUEUE_SIZE{4};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  board.power_monitor1().set_current_measurement_time_approximate(1ms);
  board.power_monitor1().set_voltage_measurement_time_approximate(200us);
  board.power_monitor1().set_sample_averaging_approximate(16);

  board.power_monitor2().set_current_measurement_time_approximate(1ms);
  board.power_monitor2().set_voltage_measurement_time_approximate(200us);
  board.power_monitor2().set_sample_averaging_approximate(16);

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(monitor_power<ClockType>(board.power_monitor1(), power_monitor1, 1000ms));
  scheduler.add_task(monitor_power<ClockType>(board.power_monitor2(), power_monitor2, 1000ms));
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN, 500ms));

  scheduler.start();
}
