#include <cstring>

#include "hal/boards/nucleo_l452re/board_nucleo_l452re.h"
#include "hal/gpio/gpio.h"

using BoardType = tvsc::hal::boards::nucleo_l452re::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  static constexpr bool demo_speed_changes{true};
  bool at_max_speed{false};
  if constexpr (demo_speed_changes) {
    board.rcc().set_clock_to_min_speed();
  }

  // Turn on clocks for the GPIO ports that we want.
  board.rcc().enable_gpio_port(BoardType::GREEN_LED_PORT);

  board.gpio<BoardType::GREEN_LED_PORT>().set_pin_mode(BoardType::GREEN_LED_PIN,
                                                       PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    for (int i = 0; i < 2; ++i) {
      board.gpio<BoardType::GREEN_LED_PORT>().toggle_pin(BoardType::GREEN_LED_PIN);

      board.clock().sleep_ms(500);

      board.gpio<BoardType::GREEN_LED_PORT>().toggle_pin(BoardType::GREEN_LED_PIN);

      board.clock().sleep_ms(500);
    }

    // Swap speeds so that we can detect any behavior changes due to the speed in the registers, LED
    // blinking, etc.
    if constexpr (demo_speed_changes) {
      if (at_max_speed) {
        board.rcc().set_clock_to_min_speed();
        at_max_speed = false;
      } else {
        board.rcc().set_clock_to_max_speed();
        at_max_speed = true;
      }
    }
  }
}
