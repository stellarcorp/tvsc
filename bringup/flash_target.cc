#include "bringup/flash_target.h"

#include "base/initializer.h"
#include "system/system.h"

using BoardType = tvsc::system::System::BoardType;

using namespace tvsc::bringup;
using namespace tvsc::system;

int main(int argc, char *argv[]) {
  tvsc::initialize(&argc, &argv);

  BoardType &board{BoardType::board()};

  System::scheduler().add_task(flash_target(
      board.programmer(), board.gpio<BoardType::DEBUG_LED_PORT>(), BoardType::DEBUG_LED_PIN));
  System::scheduler().start();
}
