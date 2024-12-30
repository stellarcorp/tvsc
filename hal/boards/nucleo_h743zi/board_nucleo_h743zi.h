#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/boards/boards.h"
#include "hal/boards/nucleo_h743zi/clock.h"
#include "hal/boards/nucleo_h743zi/gpio.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/pins.h"
#include "hal/time/clock.h"

extern "C" {
#include "stm32h7xx.h"
}

namespace tvsc::hal::boards::nucleo_h743zi {

class Board final {
 public:
  static constexpr Boards BOARD_ID{Boards::NUCLEO_STM32H743ZI};
  static constexpr gpio::Port NUM_GPIO_PORTS{4};

 private:
  ClockStm32H7xx clock_{};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32H7xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first four here, but this can be expanded if necessary.
  GpioStm32H7xx gpio_port_a_{GPIOA};
  GpioStm32H7xx gpio_port_b_{GPIOB};
  GpioStm32H7xx gpio_port_c_{GPIOC};
  GpioStm32H7xx gpio_port_d_{GPIOD};

 public:
  template <gpio::Port GPIO_PORT>
  gpio::Gpio& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    if constexpr (GPIO_PORT == 0) {
      return gpio_port_a_;
    }
    if constexpr (GPIO_PORT == 1) {
      return gpio_port_b_;
    }
    if constexpr (GPIO_PORT == 2) {
      return gpio_port_c_;
    }
    if constexpr (GPIO_PORT == 3) {
      return gpio_port_d_;
    }
  }

  time::Clock& clock() { return clock_; }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
