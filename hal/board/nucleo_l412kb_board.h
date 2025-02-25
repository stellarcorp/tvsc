#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/adc/adc.h"
#include "hal/adc/stm32l4xx_adc.h"
#include "hal/dma/dma.h"
#include "hal/dma/stm32l4xx_dma.h"
#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/stm_gpio.h"
#include "hal/power/power.h"
#include "hal/power/stm32l4xx_power.h"
#include "hal/random/rng.h"
#include "hal/random/stm32xxxx_rng.h"
#include "hal/rcc/rcc.h"
#include "hal/rcc/stm32l4xx_rcc.h"
#include "hal/stm32_peripheral_ids.h"
#include "hal/time/clock.h"
#include "hal/time/embedded_clock.h"
#include "hal/timer/stm32l4xx_timer.h"
#include "hal/timer/timer.h"
#include "hal/watchdog/stm32l4xx_watchdog.h"
#include "hal/watchdog/watchdog.h"
#include "third_party/stm32/stm32.h"

namespace tvsc::hal::board {

class Board final {
 public:
  static constexpr gpio::Port NUM_GPIO_PORTS{6};
  static constexpr size_t NUM_DAC_CHANNELS{0};
  static constexpr size_t NUM_DEBUG_LEDS{1};

  static constexpr gpio::Port GPIO_PORT_A{0};
  static constexpr gpio::Port GPIO_PORT_B{1};
  static constexpr gpio::Port GPIO_PORT_C{2};
  static constexpr gpio::Port GPIO_PORT_H{7};

  // Location of the LEDs provided by this board.
  static constexpr gpio::Port GREEN_LED_PORT{GPIO_PORT_B};
  static constexpr gpio::Pin GREEN_LED_PIN{3};

  static constexpr std::array<gpio::Port, NUM_DEBUG_LEDS> DEBUG_LED_PORTS{GREEN_LED_PORT};
  static constexpr std::array<gpio::Pin, NUM_DEBUG_LEDS> DEBUG_LED_PINS{GREEN_LED_PIN};

 private:
  rcc::RccStm32L4xx rcc_{};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32L4xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  gpio::GpioStm32xxxx gpio_port_a_{reinterpret_cast<void*>(GPIOA_BASE), 0};
  gpio::GpioStm32xxxx gpio_port_b_{reinterpret_cast<void*>(GPIOB_BASE), 1};
  gpio::GpioStm32xxxx gpio_port_c_{reinterpret_cast<void*>(GPIOC_BASE), 2};
  gpio::GpioStm32xxxx gpio_port_h_{reinterpret_cast<void*>(GPIOH_BASE), 7};
  // Don't forget to modify NUM_GPIO_PORTS and add a GPIO_PORT_* above.

  power::PowerStm32L4xx power_{};

  dma::DmaStm32l4xx dma_{Stm32PeripheralIds::DMA1_CHANNEL1_ID, DMA1_Channel1, DMA_REQUEST_0};
  adc::AdcStm32l4xx adc_{ADC1, dma_};

  timer::TimerStm32l4xx timer2_{Stm32PeripheralIds::TIM2_ID, TIM2};

  rcc::LsiOscillatorStm32L4xx lsi_oscillator_{};
  timer::Stm32l4xxLptim lptim1_{Stm32PeripheralIds::LPTIM1_ID, LPTIM1, lsi_oscillator_};

  time::EmbeddedClock clock_{lptim1_, power_, rcc_};

  rcc::Hsi48OscillatorStm32L4xx hsi48_oscillator_{};
  random::RngStm32xxxx rng_{hsi48_oscillator_};

  watchdog::WatchdogStm32l4xx iwdg_{IWDG, lsi_oscillator_};

  // Note that these GPIO Ports are disallowed on this board. They are marked private to make it
  // more difficult to accidentally use them.
  static constexpr gpio::Port GPIO_PORT_D{3};
  static constexpr gpio::Port GPIO_PORT_E{4};
  static constexpr gpio::Port GPIO_PORT_F{5};
  static constexpr gpio::Port GPIO_PORT_G{6};
  static constexpr size_t NUM_DISALLOWED_PORTS{4};

  static Board board_;

  // Private constructor to restrict inadvertent instantiation and copying.
  Board();

 public:
  // One board per executable.
  static Board& board();

  template <gpio::Port GPIO_PORT>
  gpio::GpioPeripheral& gpio() {
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

  gpio::GpioPeripheral& gpio(gpio::Port port) {
    if (port == 0) {
      return gpio_port_a_;
    } else if (port == 1) {
      return gpio_port_b_;
    } else if (port == 2) {
      return gpio_port_c_;
    }
    error();
  }

  time::Clock& clock() { return clock_; }

  rcc::Rcc& rcc() { return rcc_; };

  power::Power& power() { return power_; }

  adc::AdcPeripheral& adc() { return adc_; }

  dma::DmaPeripheral& dma() { return dma_; }

  timer::TimerPeripheral& timer2() { return timer2_; }
  timer::TimerPeripheral& lptim1() { return lptim1_; }

  random::RngPeripheral& rng() { return rng_; }

  watchdog::WatchdogPeripheral& iwdg() { return iwdg_; }
};

}  // namespace tvsc::hal::board
