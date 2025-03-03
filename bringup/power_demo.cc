#include <cstdint>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"
#include "time/embedded_clock.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType& board{BoardType::board()};
  auto& rcc{board.rcc()};

  static constexpr bool demo_speed_changes{true};
  bool at_max_speed{false};
  if constexpr (demo_speed_changes) {
    rcc.set_clock_to_min_speed();
  }

  // Turn on clocks for the GPIO ports that we want.
  auto gpio{board.gpio<BoardType::GREEN_LED_PORT>().access()};

  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    for (int i = 0; i < 20; ++i) {
      gpio.toggle_pin(BoardType::GREEN_LED_PIN);
      tvsc::time::EmbeddedClock& clock{tvsc::time::EmbeddedClock::clock()};
      clock.sleep_ms(75);
    }

    // Swap speeds so that we can detect any behavior changes.
    if constexpr (demo_speed_changes) {
      if (at_max_speed) {
        rcc.set_clock_to_min_speed();
        at_max_speed = false;
      } else {
        rcc.set_clock_to_max_speed();
        at_max_speed = true;
      }
    }
  }
}
