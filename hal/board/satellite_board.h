#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/board/basic_board.h"
#include "hal/error.h"
#include "hal/imu/bmi323_imu.h"
#include "hal/imu/imu.h"
#include "hal/led/hal_led.h"
#include "hal/led/led.h"
#include "hal/mcu/mcu.h"
#include "hal/mcu/stm32l4xx.h"
#include "hal/power_monitor/ina260_power_monitor.h"
#include "hal/power_monitor/power_monitor.h"
#include "hal/programmer/programmer.h"
#include "hal/programmer/stm32l4xx_programmer.h"
#include "hal/stm32_peripheral_ids.h"
#include "third_party/stm32/stm32.h"

namespace tvsc::hal::board {

class Board final {
 public:
  using PinoutType = pinout::Pinout;

 private:
  std::array<led::HalLed, PinoutType::NUM_LEDS> LEDS{
      led::HalLed(mcu().create_peripheral(PinoutType::LED_PINS[0])),
  };

  // imu::Bmi323Imu imu1_{0x68, mcu().i2c<0>()};
  // imu::Bmi323Imu imu2_{0x69, mcu().i2c<1>()};
  // power_monitor::Ina260PowerMonitor power_monitor1_{0x40, mcu().i2c<2>()};
  // power_monitor::Ina260PowerMonitor power_monitor2_{0x41, mcu().i2c<2>()};

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

  template <size_t LED>
  led::LedPeripheral& led() noexcept {
    static_assert(LED < PinoutType::NUM_LEDS);
    return LEDS[LED];
  }

  led::LedPeripheral& led(size_t led_number) noexcept { return LEDS.at(led_number); }

  auto& debug_led() noexcept { return led<0>(); }

  programmer::ProgrammerPeripheral& programmer() { return programmer_; }

  // imu::ImuPeripheral& imu1() { return imu1_; }
  // imu::ImuPeripheral& imu2() { return imu2_; }

  // power_monitor::PowerMonitorPeripheral& power_monitor1() { return power_monitor1_; }
  // power_monitor::PowerMonitorPeripheral& power_monitor2() { return power_monitor2_; }
};

static_assert(BasicBoard<Board>);

}  // namespace tvsc::hal::board
