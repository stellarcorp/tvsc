#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/stm_gpio.h"
#include "hal/pinout/basic_pinout.h"
#include "third_party/stm32/stm32.h"

namespace tvsc::hal::pinout {

class Pinout final {
 public:
  static constexpr size_t NUM_GPIO_PORTS{6};
  static constexpr size_t NUM_I2C_BUSES{3};
  static constexpr size_t NUM_CAN_BUSES{1};
  static constexpr size_t NUM_DAC_CHANNELS{1};
  static constexpr size_t NUM_ADC_CHANNELS{1};
  static constexpr size_t NUM_DEBUG_LEDS{1};

  static constexpr gpio::PortNumber GPIO_PORT_A{0};
  static constexpr gpio::PortNumber GPIO_PORT_B{1};
  static constexpr gpio::PortNumber GPIO_PORT_C{2};
  static constexpr gpio::PortNumber GPIO_PORT_D{3};
  static constexpr gpio::PortNumber GPIO_PORT_E{4};
  static constexpr gpio::PortNumber GPIO_PORT_H{7};

  static constexpr std::array<gpio::PinRef, NUM_DAC_CHANNELS> DAC_CHANNEL_PINS{
      gpio::PinRef{GPIO_PORT_A, 4},
  };

  // Location of the pins to read the board id.
  static constexpr gpio::PinRef BOARD_ID_POWER_PIN{GPIO_PORT_A, 6};
  static constexpr gpio::PinRef BOARD_ID_SENSE_PIN{GPIO_PORT_A, 7};

  // Debug LEDs provided by this board.
  static constexpr std::array<gpio::PinRef, NUM_DEBUG_LEDS> DEBUG_LED_PINS{
      gpio::PinRef{GPIO_PORT_C, 13},
  };

  static constexpr gpio::PinRef I2C1_SCL_PIN{GPIO_PORT_B, 6};
  static constexpr gpio::PinRef I2C1_SDA_PIN{GPIO_PORT_B, 7};
  static constexpr gpio::PinRef I2C2_SCL_PIN{GPIO_PORT_B, 10};
  static constexpr gpio::PinRef I2C2_SDA_PIN{GPIO_PORT_B, 11};
  static constexpr gpio::PinRef I2C3_SCL_PIN{GPIO_PORT_C, 0};
  static constexpr gpio::PinRef I2C3_SDA_PIN{GPIO_PORT_C, 1};

  static constexpr gpio::PinRef CAN1_TX_PIN{GPIO_PORT_A, 12};
  static constexpr gpio::PinRef CAN1_RX_PIN{GPIO_PORT_A, 11};
  static constexpr gpio::PinRef CAN1_SHUTDOWN_PIN{GPIO_PORT_A, 9};
  static constexpr gpio::PinRef CAN1_SILENT_PIN{GPIO_PORT_A, 10};

  static constexpr gpio::PinRef PROGRAMMER_SWDIO_CONTROL_PIN{GPIO_PORT_B, 15};
  static constexpr gpio::PinRef PROGRAMMER_SWCLK_CONTROL_PIN{GPIO_PORT_B, 13};
  static constexpr gpio::PinRef PROGRAMMER_NRST_CONTROL_PIN{GPIO_PORT_B, 14};
};

static_assert(BasicPinout<Pinout>);

}  // namespace tvsc::hal::pinout
