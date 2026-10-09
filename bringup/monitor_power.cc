#include "bringup/monitor_power.h"

#include <array>
#include <chrono>

#include "base/constexpr_for.h"
#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/watchdog.h"
#include "hal/pinout/pinout.h"
#include "platforms/linker_sections.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;

static constexpr size_t NUM_POWER_MONITORS{tvsc::hal::pinout::Pinout::NUM_POWER_MONITORS};

PRINCIPAL_RESULT std::array<PowerUsage, NUM_POWER_MONITORS> power_monitors{};

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  auto& mcu{System::mcu()};
  auto& board{System::board()};

  tvsc::constexpr_for<0, NUM_POWER_MONITORS>([&](auto index) {
    board.power_monitor<index>().set_current_measurement_time_approximate(1ms);
    board.power_monitor<index>().set_voltage_measurement_time_approximate(200us);
    board.power_monitor<index>().set_sample_averaging_approximate(16);
  });

  auto& scheduler{System::scheduler()};
  tvsc::constexpr_for<0, NUM_POWER_MONITORS>([&](auto index) {
    scheduler.add_task(monitor_power(board.power_monitor<index>(), power_monitors[index], 1000ms));
  });
  scheduler.add_task(blink(board.debug_led()));
  scheduler.add_task(run_watchdog(mcu.iwdg()));

  scheduler.start();
}
