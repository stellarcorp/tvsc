#include "bringup/monitor_power.h"

#include <chrono>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/watchdog.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;

extern "C" {

__attribute__((section(".status.value"))) PowerUsage power_monitor1{};
__attribute__((section(".status.value"))) PowerUsage power_monitor2{};

}  // extern "C"

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& board{System::board()};

  board.power_monitor1().set_current_measurement_time_approximate(1ms);
  board.power_monitor1().set_voltage_measurement_time_approximate(200us);
  board.power_monitor1().set_sample_averaging_approximate(16);

  board.power_monitor2().set_current_measurement_time_approximate(1ms);
  board.power_monitor2().set_voltage_measurement_time_approximate(200us);
  board.power_monitor2().set_sample_averaging_approximate(16);

  auto& scheduler{System::scheduler()};
  scheduler.add_task(monitor_power(board.power_monitor1(), power_monitor1, 1000ms));
  scheduler.add_task(monitor_power(board.power_monitor2(), power_monitor2, 1000ms));
  scheduler.add_task(blink(board.gpio<System::BoardType::DEBUG_LED_PORT>(),
                           System::BoardType::DEBUG_LED_PIN, 500ms));
  scheduler.add_task(run_watchdog(board.iwdg()));

  scheduler.start();
}
