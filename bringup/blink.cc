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
}
