#pragma once

#include <cstddef>
#include <cstdint>

#include "hal/adc/adc.h"
#include "hal/adc/stm32l4xx_adc.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/stm_gpio.h"
#include "hal/power/power.h"
#include "hal/power/stm32l4xx_power.h"
#include "hal/rcc/rcc.h"
#include "hal/rcc/stm32l4xx_rcc.h"
#include "hal/time/clock.h"
#include "hal/time/stm_clock.h"
#include "third_party/stm32/stm32.h"

namespace tvsc::hal::board {

class Board final {
 public:
  static constexpr gpio::Port NUM_GPIO_PORTS{6};

  static constexpr gpio::Port GPIO_PORT_A{0};
  static constexpr gpio::Port GPIO_PORT_B{1};
  static constexpr gpio::Port GPIO_PORT_C{2};
  static constexpr gpio::Port GPIO_PORT_H{7};

  // Location of the LEDs provided by this board.
  static constexpr size_t NUM_USER_LEDS{1};
  static constexpr gpio::Port GREEN_LED_PORT{GPIO_PORT_B};
  static constexpr gpio::Pin GREEN_LED_PIN{3};

 private:
  rcc::RccStm32L4xx rcc_{reinterpret_cast<void*>(RCC_BASE), reinterpret_cast<void*>(SysTick_BASE),
                         reinterpret_cast<void*>(ADC1_BASE)};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32L4xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  gpio::GpioStm32xxxx gpio_port_a_{reinterpret_cast<void*>(GPIOA)};
  gpio::GpioStm32xxxx gpio_port_b_{reinterpret_cast<void*>(GPIOB)};
  gpio::GpioStm32xxxx gpio_port_c_{reinterpret_cast<void*>(GPIOC)};
  gpio::GpioStm32xxxx gpio_port_h_{reinterpret_cast<void*>(GPIOH)};
  // Don't forget to modify NUM_GPIO_PORTS and add a GPIO_PORT_* above.

  power::PowerStm32L4xx power_{reinterpret_cast<void*>(PWR_BASE)};

  time::ClockStm32xxxx clock_{};

  adc::AdcStm32l4xx adc_{reinterpret_cast<void*>(ADC1_BASE)};

  // Note that these GPIO Ports are disallowed on this board. They are marked private to make it
  // more difficult to accidentally use them.
  static constexpr gpio::Port GPIO_PORT_D{3};
  static constexpr gpio::Port GPIO_PORT_E{4};
  static constexpr gpio::Port GPIO_PORT_F{5};
  static constexpr gpio::Port GPIO_PORT_G{6};
  static constexpr size_t NUM_DISALLOWED_PORTS{4};

 public:
  template <gpio::Port GPIO_PORT>
  gpio::Gpio& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS + NUM_DISALLOWED_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    static_assert(
        GPIO_PORT != GPIO_PORT_D,
        "Invalid GPIO port id. Port D does not exist on this board. Likely, there is a mismatch in "
        "the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    static_assert(
        GPIO_PORT != GPIO_PORT_E,
        "Invalid GPIO port id. Port E does not exist on this board. Likely, there is a mismatch in "
        "the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    static_assert(
        GPIO_PORT != GPIO_PORT_F,
        "Invalid GPIO port id. Port F does not exist on this board. Likely, there is a mismatch in "
        "the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    static_assert(
        GPIO_PORT != GPIO_PORT_G,
        "Invalid GPIO port id. Port G does not exist on this board. Likely, there is a mismatch in "
        "the build that instantiates a Board "
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
    if constexpr (GPIO_PORT == 7) {
      return gpio_port_h_;
    }
  }

  time::Clock& clock() { return clock_; }

  rcc::Rcc& rcc() { return rcc_; };

  power::Power& power() { return power_; }

  adc::Adc& adc() { return adc_; }
};

}  // namespace tvsc::hal::board
