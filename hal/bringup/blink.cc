#include <cstring>

#include "hal/board/board.h"
#include "hal/gpio/gpio.h"

using BoardType = tvsc::hal::board::Board;

using namespace tvsc::hal::gpio;

int main() {
  BoardType board{};

  // Turn on clocks for the GPIO ports that we want.
  board.rcc().enable_gpio_port_clock(BoardType::GREEN_LED_PORT);

  auto& gpio{board.gpio<BoardType::GREEN_LED_PORT>()};
  gpio.set_pin_mode(BoardType::GREEN_LED_PIN, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  while (true) {
    auto& clock{board.clock()};

    gpio.toggle_pin(BoardType::GREEN_LED_PIN);
    clock.sleep_ms(500);
    gpio.toggle_pin(BoardType::GREEN_LED_PIN);
    clock.sleep_ms(500);
  }
}
