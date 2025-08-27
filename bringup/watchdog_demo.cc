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

  using BoardType = tvsc::system::System::BoardType;

  // Sleep now so that we can detect the board reset. If we just launch into the tasks, we might not
  // be able to detect that the watchdog caused a reset.
  System::clock().sleep(CYCLE_TIME);

  System::scheduler().add_task(run_watchdog(System::board().iwdg()));
  System::scheduler().add_task(
      blink(System::board().gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  System::scheduler().add_task(quit(CYCLE_TIME));
  System::scheduler().start();
}
