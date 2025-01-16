#include <cstring>

#include "hal/boards/nucleo_h743zi/board_nucleo_h743zi.h"
#include "hal/gpio/gpio.h"

using BoardType = tvsc::hal::boards::nucleo_h743zi::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  board.rcc().set_clock_to_min_speed();
  bool at_max_speed{false};

  // Turn on clocks for the GPIO ports that we want.
  board.rcc().enable_gpio_port(BoardType::RED_LED_PORT);
  board.rcc().enable_gpio_port(BoardType::YELLOW_LED_PORT);
  board.rcc().enable_gpio_port(BoardType::GREEN_LED_PORT);

  board.gpio<BoardType::RED_LED_PORT>().set_pin_mode(BoardType::RED_LED_PIN,
                                                     PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  board.gpio<BoardType::YELLOW_LED_PORT>().set_pin_mode(BoardType::YELLOW_LED_PIN,
                                                        PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  board.gpio<BoardType::GREEN_LED_PORT>().set_pin_mode(BoardType::GREEN_LED_PIN,
                                                       PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    for (int i = 0; i < 2; ++i) {
      board.gpio<BoardType::RED_LED_PORT>().toggle_pin(BoardType::RED_LED_PIN);
      board.gpio<BoardType::YELLOW_LED_PORT>().toggle_pin(BoardType::YELLOW_LED_PIN);

      board.clock().sleep_ms(750);

      board.gpio<BoardType::YELLOW_LED_PORT>().toggle_pin(BoardType::YELLOW_LED_PIN);
      board.gpio<BoardType::GREEN_LED_PORT>().toggle_pin(BoardType::GREEN_LED_PIN);

      board.clock().sleep_ms(750);

      board.gpio<BoardType::RED_LED_PORT>().toggle_pin(BoardType::RED_LED_PIN);
      board.gpio<BoardType::YELLOW_LED_PORT>().toggle_pin(BoardType::YELLOW_LED_PIN);

      board.clock().sleep_ms(750);

      board.gpio<BoardType::YELLOW_LED_PORT>().toggle_pin(BoardType::YELLOW_LED_PIN);
      board.gpio<BoardType::GREEN_LED_PORT>().toggle_pin(BoardType::GREEN_LED_PIN);

      board.clock().sleep_ms(750);
    }

    // Swap speeds so that we can detect any behavior changes due to the speed in the registers, LED
    // blinking, etc.
    if (at_max_speed) {
      board.rcc().set_clock_to_min_speed();
      at_max_speed = false;
    } else {
      board.rcc().set_clock_to_max_speed();
      at_max_speed = true;
    }
  }
}
