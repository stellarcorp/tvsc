#include "bringup/random_blink.h"

#include "base/initializer.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  System::scheduler().add_task(blink_randomly(System::board().debug_led<0>()));

  System::scheduler().start();
}
