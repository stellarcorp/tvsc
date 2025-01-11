#include "hal/boards/nucleo_h743zi/board_nucleo_h743zi.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/pins.h"

using BoardType = tvsc::hal::boards::nucleo_h743zi::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  // Configure the push button as an input.
  board.gpio<BoardType::BLUE_PUSH_BUTTON_PORT>().set_pin_mode(BoardType::BLUE_PUSH_BUTTON_PIN,
                                                              PinMode::INPUT_FLOATING);

  /* Configure the debug LEDs. */
  board.gpio<BoardType::RED_LED_PORT>().set_pin_mode(BoardType::RED_LED_PIN,
                                                     PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  board.gpio<BoardType::YELLOW_LED_PORT>().set_pin_mode(BoardType::YELLOW_LED_PIN,
                                                        PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  board.gpio<BoardType::GREEN_LED_PORT>().set_pin_mode(BoardType::GREEN_LED_PIN,
                                                       PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    board.gpio<BoardType::GREEN_LED_PORT>().toggle_pin(BoardType::GREEN_LED_PIN);

    board.clock().sleep_ms(750);
  }
}
