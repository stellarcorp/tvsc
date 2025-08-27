#include "bringup/monitor_imu.h"

#include <chrono>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/watchdog.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;

extern "C" {

__attribute__((section(".status.value"))) ImuReading imu1_reading{};
__attribute__((section(".status.value"))) ImuReading imu2_reading{};

}  // extern "C"

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{System::board()};
  auto& scheduler{System::scheduler()};

  scheduler.add_task(monitor_imu(board.imu1(), imu1_reading, 1ms));
  // scheduler.add_task(monitor_imu<ClockType>(board.imu2(), imu2_reading,
  // 1ms));
  scheduler.add_task(blink(board.gpio<System::BoardType::DEBUG_LED_PORT>(),
                           System::BoardType::DEBUG_LED_PIN, 500ms));
  scheduler.add_task(run_watchdog(board.iwdg()));

  scheduler.start();
}
