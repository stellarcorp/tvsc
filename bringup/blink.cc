/**
 * Variant of the standard blink program that uses the scheduler to run a task to blink the debug
 * LED. This variant requires several subsystems to work correctly. Beyond GPIO and a systick clock,
 * it requires the RCC to support configurable clock speeds, and it requies the low-power timer
 * (LPTIM) be usable. For a more standard, trivial blink program, look for a bringup script based on
 * systick.
 */
#include "bringup/blink.h"

#include "base/initializer.h"
#include "bringup/quit.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  System::scheduler().add_task(blink(System::board().debug_led()));
  System::scheduler().add_task(quit());

  System::scheduler().start();

  return 0xcaca;
}
