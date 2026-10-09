/**
 * Script to demonstrate the watchdog functionality. The expected behavior should be that it does
 * nothing (sleeps) at first, blinks the debug LED for a while, and then it turns off.
 *
 * When the script starts, it just sleeps. This first step allows us to recognize when the board
 * gets reset due to an unfed watchdog timer.
 */
#include <chrono>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "bringup/quit.h"
#include "bringup/watchdog.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  static constexpr auto CYCLE_TIME{5s};

  // Sleep now so that we can detect the board reset. If we just launch into the tasks, we might not
  // be able to detect that the watchdog caused a reset.
  System::clock().sleep(CYCLE_TIME);

  System::scheduler().add_task(run_watchdog(System::mcu().iwdg()));
  System::scheduler().add_task(blink(System::board().debug_led()));
  System::scheduler().add_task(quit(CYCLE_TIME));
  System::scheduler().start();
}
