#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/adc/adc.h"
#include "hal/adc/stm32l4xx_adc.h"
#include "hal/board/basic_board.h"
#include "hal/can_bus/can_bus.h"
#include "hal/can_bus/stm32l4xx_can_bus.h"
#include "hal/dac/dac.h"
#include "hal/dac/stm32xxxx_dac.h"
#include "hal/dma/dma.h"
#include "hal/dma/stm32l4xx_dma.h"
#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/stm_gpio.h"
#include "hal/i2c/i2c.h"
#include "hal/i2c/stm32l4xx_i2c.h"
#include "hal/imu/bmi323_imu.h"
#include "hal/imu/imu.h"
#include "hal/mcu/mcu.h"
#include "hal/mcu/stm32l4xx.h"
#include "hal/mcu_identification/mcu_identification.h"
#include "hal/mcu_identification/stm32l4xx_mcu_identification.h"
#include "hal/power/power.h"
#include "hal/power/stm32l4xx_power.h"
#include "hal/power_monitor/ina260_power_monitor.h"
#include "hal/power_monitor/power_monitor.h"
#include "hal/programmer/programmer.h"
#include "hal/programmer/stm32l4xx_programmer.h"
#include "hal/random/rng.h"
#include "hal/random/stm32xxxx_rng.h"
#include "hal/rcc/rcc.h"
#include "hal/rcc/stm32l4xx_rcc.h"
#include "hal/stm32_peripheral_ids.h"
#include "hal/systick/stm32l4xx_systick.h"
#include "hal/systick/systick.h"
#include "hal/timer/stm32l4xx_timer.h"
#include "hal/timer/timer.h"
#include "hal/watchdog/stm32l4xx_watchdog.h"
#include "hal/watchdog/watchdog.h"
#include "third_party/stm32/stm32.h"

namespace tvsc::hal::board {

class Board final {
 public:
  // Location of the pins to read the board id.
  static constexpr gpio::PortNumber BOARD_ID_POWER_PORT{mcu::Mcu::GPIO_PORT_A};
  static constexpr gpio::PortNumber BOARD_ID_SENSE_PORT{mcu::Mcu::GPIO_PORT_A};
  static constexpr gpio::PinNumber BOARD_ID_POWER_PIN{6};
  static constexpr gpio::PinNumber BOARD_ID_SENSE_PIN{7};

  // Debug LEDs provided by this board.
  static constexpr size_t NUM_DEBUG_LEDS{1};
  static constexpr gpio::PortNumber DEBUG_LED_PORT{mcu::Mcu::GPIO_PORT_C};
  static constexpr gpio::PinNumber DEBUG_LED_PIN{13};

  static constexpr std::array<gpio::PortNumber, NUM_DEBUG_LEDS> DEBUG_LED_PORTS{DEBUG_LED_PORT};
  static constexpr std::array<gpio::PinNumber, NUM_DEBUG_LEDS> DEBUG_LED_PINS{DEBUG_LED_PIN};

 private:
  std::array<gpio::PinPeripheral, NUM_DEBUG_LEDS> DEBUG_LEDS{
      gpio::PinPeripheral{mcu().gpio<DEBUG_LED_PORTS[0]>(), DEBUG_LED_PINS[0]},
  };

  imu::Bmi323Imu imu1_{0x68, mcu().i2c<0>()};
  imu::Bmi323Imu imu2_{0x69, mcu().i2c<1>()};

  power_monitor::Ina260PowerMonitor power_monitor1_{0x40, mcu().i2c<2>()};
  power_monitor::Ina260PowerMonitor power_monitor2_{0x41, mcu().i2c<2>()};

  programmer::ProgrammerStm32l4xx programmer_{mcu().gpio<1>(),             //
                                              /* SWDIO_CONTROL Pin */ 15,  //
                                              /* SWCLK_CONTROL Pin */ 13,  //
                                              /* NRST_CONTROL Pin */ 14};

  static Board board_;

  // Private constructor to restrict inadvertent instantiation and copying.
  Board() = default;

 public:
  // One board per executable.
  static Board& board();

  static mcu::Mcu& mcu();

  template <size_t LED = 0>
  gpio::PinPeripheral& debug_led() noexcept {
    static_assert(LED < NUM_DEBUG_LEDS);
    return DEBUG_LEDS[LED];
  }
  auto& debug_led() noexcept { return debug_led<>(); }

  gpio::PinPeripheral& debug_led(size_t led_number) noexcept { return DEBUG_LEDS.at(led_number); }

  programmer::ProgrammerPeripheral& programmer() { return programmer_; }

  imu::ImuPeripheral& imu1() { return imu1_; }
  imu::ImuPeripheral& imu2() { return imu2_; }

  power_monitor::PowerMonitorPeripheral& power_monitor1() { return power_monitor1_; }
  power_monitor::PowerMonitorPeripheral& power_monitor2() { return power_monitor2_; }
};

static_assert(BasicBoard<Board>);

}  // namespace tvsc::hal::board
