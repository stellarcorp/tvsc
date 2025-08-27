#include <chrono>
#include <cstring>

#include "base/initializer.h"
#include "bringup/blink.h"
#include "system/system.h"

using BoardType = tvsc::system::System::BoardType;

using namespace tvsc::bringup;
using namespace tvsc::system;
using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  tvsc::initialize(&argc, &argv);

  static constexpr auto BASE_DURATION{200ms};
  static constexpr int DURATION_MULTIPLES[] = {4, 3, 2};
  static_assert(BoardType::NUM_DEBUG_LEDS < 4, "Need to implement blink for more LEDs");

  for (size_t i = 0; i < BoardType::NUM_DEBUG_LEDS; ++i) {
    System::scheduler().add_task(
        blink(System::board().debug_led(i), DURATION_MULTIPLES[i] * BASE_DURATION));
  }

  System::scheduler().start();
}
