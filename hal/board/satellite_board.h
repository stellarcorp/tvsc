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
#include "hal/led/hal_led.h"
#include "hal/led/led.h"
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
  using PinoutType = pinout::Pinout;

 private:
  std::array<led::HalLed, PinoutType::NUM_DEBUG_LEDS> DEBUG_LEDS{
      led::HalLed(mcu().create_peripheral(PinoutType::DEBUG_LED_PINS[0])),
  };

  imu::Bmi323Imu imu1_{0x68, mcu().i2c<0>()};
  imu::Bmi323Imu imu2_{0x69, mcu().i2c<1>()};

  power_monitor::Ina260PowerMonitor power_monitor1_{0x40, mcu().i2c<2>()};
  power_monitor::Ina260PowerMonitor power_monitor2_{0x41, mcu().i2c<2>()};

  programmer::ProgrammerStm32l4xx programmer_{
      mcu().create_peripheral(PinoutType::PROGRAMMER_SWDIO_CONTROL_PIN),
      mcu().create_peripheral(PinoutType::PROGRAMMER_SWCLK_CONTROL_PIN),
      mcu().create_peripheral(PinoutType::PROGRAMMER_NRST_CONTROL_PIN),
  };

  static Board board_;

  // Private constructor to restrict inadvertent instantiation and copying.
  Board() = default;

 public:
  // One board per executable.
  static Board& board();

  static mcu::Mcu& mcu();

  template <size_t LED = 0>
  led::LedPeripheral& debug_led() noexcept {
    static_assert(LED < PinoutType::NUM_DEBUG_LEDS);
    return DEBUG_LEDS[LED];
  }
  auto& debug_led() noexcept { return debug_led<>(); }

  led::LedPeripheral& debug_led(size_t led_number) noexcept { return DEBUG_LEDS.at(led_number); }

  programmer::ProgrammerPeripheral& programmer() { return programmer_; }

  imu::ImuPeripheral& imu1() { return imu1_; }
  imu::ImuPeripheral& imu2() { return imu2_; }

  power_monitor::PowerMonitorPeripheral& power_monitor1() { return power_monitor1_; }
  power_monitor::PowerMonitorPeripheral& power_monitor2() { return power_monitor2_; }
};

static_assert(BasicBoard<Board>);

}  // namespace tvsc::hal::board
