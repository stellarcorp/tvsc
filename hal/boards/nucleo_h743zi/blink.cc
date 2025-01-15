#include <cstring>

#include "hal/boards/nucleo_h743zi/board_nucleo_h743zi.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/pins.h"

using BoardType = tvsc::hal::boards::nucleo_h743zi::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  board.gpio<BoardType::RED_LED_PORT>().set_pin_mode(BoardType::RED_LED_PIN,
                                                     PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  board.gpio<BoardType::YELLOW_LED_PORT>().set_pin_mode(BoardType::YELLOW_LED_PIN,
                                                        PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
  board.gpio<BoardType::GREEN_LED_PORT>().set_pin_mode(BoardType::GREEN_LED_PIN,
                                                       PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
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
}
