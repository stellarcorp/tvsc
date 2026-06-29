#include "bringup/flash_target.h"

#include "base/initializer.h"
#include "system/system.h"

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  auto &board{System::board()};

  System::scheduler().add_task(flash_target(board.programmer(), board.debug_led()));
  System::scheduler().start();
}
