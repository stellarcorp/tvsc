#include "bringup/monitor_imu.h"

#include <chrono>
#include <cstring>
#include <limits>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/watchdog.h"
#include "hal/board/board.h"
#include "system/scheduler.h"
#include "system/task.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;
using ClockType = tvsc::time::EmbeddedClock;

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;

extern "C" {

__attribute__((section(".status.value"))) ImuReading imu1_reading{};
__attribute__((section(".status.value"))) ImuReading imu2_reading{};

}  // extern "C"

static constexpr size_t QUEUE_SIZE{4};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{BoardType::board()};
  auto& clock{ClockType::clock()};

  Scheduler<ClockType, QUEUE_SIZE> scheduler{board.rcc()};
  scheduler.add_task(monitor_imu<ClockType>(board.imu1(), imu1_reading, 1ms));
  // scheduler.add_task(monitor_imu<ClockType>(board.imu2(), imu2_reading, 1ms));
  scheduler.add_task(
      blink(clock, board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN, 500ms));
  scheduler.add_task(run_watchdog<ClockType>(board.iwdg()));

  scheduler.start();
}
