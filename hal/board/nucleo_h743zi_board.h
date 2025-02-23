#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/stm_gpio.h"
#include "hal/power/power.h"
#include "hal/power/stm32h7xx_power.h"
#include "hal/rcc/rcc.h"
#include "hal/rcc/stm32h7xx_rcc.h"
#include "hal/time/clock.h"
#include "hal/time/stm_clock.h"
#include "third_party/stm32/stm32.h"

namespace tvsc::hal::board {

class Board final {
 public:
  static constexpr gpio::Port NUM_GPIO_PORTS{5};
  static constexpr gpio::Port NUM_DAC_CHANNELS{2};
  static constexpr size_t NUM_DEBUG_LEDS{3};

  static constexpr gpio::Port GPIO_PORT_A{0};
  static constexpr gpio::Port GPIO_PORT_B{1};
  static constexpr gpio::Port GPIO_PORT_C{2};
  static constexpr gpio::Port GPIO_PORT_D{3};
  static constexpr gpio::Port GPIO_PORT_E{4};

  // Location of the LEDs provided by this board.
  static constexpr size_t NUM_USER_LEDS{3};
  static constexpr gpio::Port RED_LED_PORT{GPIO_PORT_B};
  static constexpr gpio::Pin RED_LED_PIN{14};
  static constexpr gpio::Port YELLOW_LED_PORT{GPIO_PORT_E};
  static constexpr gpio::Pin YELLOW_LED_PIN{1};
  static constexpr gpio::Port GREEN_LED_PORT{GPIO_PORT_B};
  static constexpr gpio::Pin GREEN_LED_PIN{0};

  static constexpr std::array<gpio::Port, NUM_DEBUG_LEDS> DEBUG_LED_PORTS{
      RED_LED_PORT, YELLOW_LED_PORT, GREEN_LED_PORT};
  static constexpr std::array<gpio::Pin, NUM_DEBUG_LEDS> DEBUG_LED_PINS{RED_LED_PIN, YELLOW_LED_PIN,
                                                                        GREEN_LED_PIN};

  // Location of the push button on this board.
  static constexpr gpio::Port BLUE_PUSH_BUTTON_PORT{GPIO_PORT_C};
  static constexpr gpio::Pin BLUE_PUSH_BUTTON_PIN{13};

 private:
  rcc::RccStm32h7xx rcc_{reinterpret_cast<void*>(RCC_BASE), reinterpret_cast<void*>(SysTick_BASE),
                         reinterpret_cast<void*>(ADC1_BASE)};

  power::PowerStm32H7xx power_{reinterpret_cast<void*>(PWR_BASE)};

  time::ClockStm32xxxx clock_{};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32H7xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  gpio::GpioStm32xxxx gpio_port_a_{reinterpret_cast<void*>(GPIOA)};
  gpio::GpioStm32xxxx gpio_port_b_{reinterpret_cast<void*>(GPIOB)};
  gpio::GpioStm32xxxx gpio_port_c_{reinterpret_cast<void*>(GPIOC)};
  gpio::GpioStm32xxxx gpio_port_d_{reinterpret_cast<void*>(GPIOD)};
  gpio::GpioStm32xxxx gpio_port_e_{reinterpret_cast<void*>(GPIOE)};
  // Don't forget to modify NUM_GPIO_PORTS and add a GPIO_PORT_* above.

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
    if constexpr (GPIO_PORT == 4) {
      return gpio_port_e_;
    }
  }

  gpio::Gpio& gpio(gpio::Port port) {
    if (port == 0) {
      return gpio_port_a_;
    } else if (port == 1) {
      return gpio_port_b_;
    } else if (port == 2) {
      return gpio_port_c_;
    } else if (port == 3) {
      return gpio_port_d_;
    } else if (port == 4) {
      return gpio_port_e_;
    }
    error(false);
  }

  time::Clock& clock() { return clock_; }

  rcc::Rcc& rcc() { return rcc_; };

  power::Power& power() { return power_; }
};

}  // namespace tvsc::hal::board
