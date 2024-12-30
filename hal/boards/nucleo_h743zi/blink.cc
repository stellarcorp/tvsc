#include "hal/boards/nucleo_h743zi/board_nucleo_h743zi.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/pins.h"

using BoardType = tvsc::hal::boards::nucleo_h743zi::Board;

static constexpr tvsc::hal::gpio::Port DEBUG_LED_GPIO_PORT{0};
static constexpr tvsc::hal::gpio::Pin DEBUG_LED_PIN{13};

int main() {
  BoardType board{};

  board.gpio<DEBUG_LED_GPIO_PORT>().set_pin_mode(DEBUG_LED_PIN,
                                                 tvsc::hal::gpio::PinMode::MODE_OUTPUT);

  while (true) {
    board.gpio<DEBUG_LED_GPIO_PORT>().toggle_pin(DEBUG_LED_PIN);

    board.clock().sleep_ms(750);
  }
}
