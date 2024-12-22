#pragma once

#include <array>

#include "hal/boards/boards.h"
#include "hal/gpio/gpio.h"

namespace tvsc::hal::boards {

template <Boards BOARD_ID>
class Board final {
 public:
  static constexpr size_t NUM_GPIO_PORTS{1};

 private:
  constexpr std::array<gpio::Gpio, NUM_GPIO_PORTS> gpio_ports_{};

 public:
  template <size_t GPIO_PORT>
  constexpr gpio::Gpio& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    return gpio_ports_[GPIO_PORT];
  }
};

}  // namespace tvsc::hal::boards
