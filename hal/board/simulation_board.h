#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/gpio_interceptor.h"
#include "hal/gpio/gpio_noop.h"
#include "hal/watchdog/watchdog.h"
#include "hal/watchdog/watchdog_interceptor.h"
#include "hal/watchdog/watchdog_noop.h"

namespace tvsc::hal::board {

class Board final {
 public:
  static constexpr gpio::Port NUM_GPIO_PORTS{1};
  static constexpr size_t NUM_DAC_CHANNELS{0};
  static constexpr size_t NUM_DEBUG_LEDS{1};

  static constexpr gpio::Port GPIO_PORT_A{0};

  // Location of the LEDs provided by this board.
  static constexpr gpio::Port GREEN_LED_PORT{GPIO_PORT_A};
  static constexpr gpio::Pin GREEN_LED_PIN{5};

  static constexpr std::array<gpio::Port, NUM_DEBUG_LEDS> DEBUG_LED_PORTS{GREEN_LED_PORT};
  static constexpr std::array<gpio::Pin, NUM_DEBUG_LEDS> DEBUG_LED_PINS{GREEN_LED_PIN};

 private:
  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32L4xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  gpio::GpioNoop gpio_noop_{};
  gpio::GpioInterceptor gpio_interceptor_{gpio_noop_};
  // Don't forget to modify NUM_GPIO_PORTS and add a GPIO_PORT_* above.

  watchdog::WatchdogNoop iwdg_noop_{};
  watchdog::WatchdogNoop iwdg_interceptor_{iwdg_noop_};

  static Board board_;

  // Private constructor to restrict inadvertent instantiation and copying.
  Board();

 public:
  // One board per executable.
  static Board& board();

  template <gpio::Port GPIO_PORT>
  gpio::GpioPeripheral& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    if constexpr (GPIO_PORT == 0) {
      return gpio_interceptor_;
    }
  }

  /**
   * Accessor for GPIO periperhals. Note that the templated version above is vastly preferred, as it
   * gives compile-time errors, as opposed to runtime errors like this method. This method should
   * only be used as to lookup GPIO ports using integer values that can be evaluated at
   * compile-time; these integer values will mainly come from STM's HAL.
   */
  gpio::GpioPeripheral& gpio(gpio::Port port) {
    if (port == 0) {
      return gpio_interceptor_;
    }
    error();
  }

  watchdog::WatchdogPeripheral& iwdg() { return iwdg_interceptor_; }
};

}  // namespace tvsc::hal::board
