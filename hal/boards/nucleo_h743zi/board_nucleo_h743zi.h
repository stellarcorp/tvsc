#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/boards/boards.h"
#include "hal/boards/nucleo_h743zi/clock.h"
#include "hal/boards/nucleo_h743zi/gpio.h"
#include "hal/boards/nucleo_h743zi/rcc.h"
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
  static constexpr gpio::Port NUM_GPIO_PORTS{5};

  static constexpr Port GPIO_PORT_A{0};
  static constexpr Port GPIO_PORT_B{1};
  static constexpr Port GPIO_PORT_C{2};
  static constexpr Port GPIO_PORT_D{3};
  static constexpr Port GPIO_PORT_E{4};

  // Location of the LEDs provided by this board.
  static constexpr Port RED_LED_PORT{GPIO_PORT_B};
  static constexpr Pin RED_LED_PIN{14};
  static constexpr Port YELLOW_LED_PORT{GPIO_PORT_E};
  static constexpr Pin YELLOW_LED_PIN{1};
  static constexpr Port GREEN_LED_PORT{GPIO_PORT_B};
  static constexpr Pin GREEN_LED_PIN{0};

  // Location of the push button on this board.
  static constexpr Port BLUE_PUSH_BUTTON_PORT{GPIO_PORT_C};
  static constexpr Pin BLUE_PUSH_BUTTON_PIN{13};

  // Location of the USB OTG (on-the-go) full-speed (as opposed to low speed or high speed) power
  // enable pin.
  static constexpr Port USB_OTG_FS_PWR_EN_PORT{GPIO_PORT_D};
  static constexpr Pin USB_OTG_FS_PWR_EN_PIN{10};

 private:
  Rcc* rcc_{RCC_BASE};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32H7xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  GpioStm32H7xx gpio_port_a_{GPIOA};
  GpioStm32H7xx gpio_port_b_{GPIOB};
  GpioStm32H7xx gpio_port_c_{GPIOC};
  GpioStm32H7xx gpio_port_d_{GPIOD};
  GpioStm32H7xx gpio_port_e_{GPIOE};
  // Don't forget to modify NUM_GPIO_PORTS above.

  ClockStm32H7xx clock_{};

 public:
  Board() {
    // Turn off all of the LEDs.
    gpio<RED_LED_PORT>().write_pin(RED_LED_PIN, 0);
    gpio<YELLOW_LED_PORT>().write_pin(YELLOW_LED_PIN, 0);
    gpio<GREEN_LED_PORT>().write_pin(GREEN_LED_PIN, 0);

    // Turn off USB OTG power. Do we need to do this?
    board.gpio<USB_OTG_FS_PWR_EN_PORT>.write_pin(USB_OTG_FS_PWR_EN_PIN, 0);

    // Do we need to configure USB OTG by default?
    // /*Configure GPIO pin : USB_OTG_FS_PWR_EN_Pin */
    // GPIO_InitStruct.Pin = USB_OTG_FS_PWR_EN_Pin;
    // GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    // HAL_GPIO_Init(USB_OTG_FS_PWR_EN_GPIO_Port, &GPIO_InitStruct);

    // /*Configure GPIO pin : USB_OTG_FS_OVCR_Pin */
    // GPIO_InitStruct.Pin = USB_OTG_FS_OVCR_Pin;
    // GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // HAL_GPIO_Init(USB_OTG_FS_OVCR_GPIO_Port, &GPIO_InitStruct);
  }

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
    if constexpr (GPIO_PORT == 4) {
      return gpio_port_e_;
    }
  }

  time::Clock& clock() { return clock_; }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
