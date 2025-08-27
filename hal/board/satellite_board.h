#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/adc/adc.h"
#include "hal/adc/stm32l4xx_adc.h"
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
#include "hal/mcu/stm32l4xx_mcu.h"
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
  static constexpr gpio::PortNumber NUM_GPIO_PORTS{6};
  static constexpr size_t NUM_DAC_CHANNELS{1};
  static constexpr size_t NUM_DEBUG_LEDS{1};

  static constexpr gpio::PortNumber GPIO_PORT_A{0};
  static constexpr gpio::PortNumber GPIO_PORT_B{1};
  static constexpr gpio::PortNumber GPIO_PORT_C{2};
  static constexpr gpio::PortNumber GPIO_PORT_D{3};
  static constexpr gpio::PortNumber GPIO_PORT_E{4};
  static constexpr gpio::PortNumber GPIO_PORT_H{7};

  // Location of the pins to read the board id.
  static constexpr gpio::PortNumber BOARD_ID_POWER_PORT{GPIO_PORT_A};
  static constexpr gpio::PortNumber BOARD_ID_SENSE_PORT{GPIO_PORT_A};
  static constexpr gpio::PinNumber BOARD_ID_POWER_PIN{6};
  static constexpr gpio::PinNumber BOARD_ID_SENSE_PIN{7};

  // Location of the LEDs provided by this board.
  static constexpr gpio::PortNumber DEBUG_LED_PORT{GPIO_PORT_C};
  static constexpr gpio::PinNumber DEBUG_LED_PIN{13};

  static constexpr std::array<gpio::PortNumber, NUM_DEBUG_LEDS> DEBUG_LED_PORTS{DEBUG_LED_PORT};
  static constexpr std::array<gpio::PinNumber, NUM_DEBUG_LEDS> DEBUG_LED_PINS{DEBUG_LED_PIN};

  static constexpr gpio::PortNumber DAC_CHANNEL_1_PORT{GPIO_PORT_A};
  static constexpr gpio::PinNumber DAC_CHANNEL_1_PIN{4};

  static constexpr std::array<gpio::PortNumber, NUM_DAC_CHANNELS> DAC_PORTS{DAC_CHANNEL_1_PORT};
  static constexpr std::array<gpio::PinNumber, NUM_DAC_CHANNELS> DAC_PINS{DAC_CHANNEL_1_PIN};

 private:
  rcc::RccStm32L4xx rcc_{};

  systick::SysTickStm32l4xx sys_tick_{};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32L4xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  gpio::GpioStm32xxxx gpio_port_a_{reinterpret_cast<void*>(GPIOA_BASE), 0};
  gpio::GpioStm32xxxx gpio_port_b_{reinterpret_cast<void*>(GPIOB_BASE), 1};
  gpio::GpioStm32xxxx gpio_port_c_{reinterpret_cast<void*>(GPIOC_BASE), 2};
  gpio::GpioStm32xxxx gpio_port_d_{reinterpret_cast<void*>(GPIOD_BASE), 3};
  gpio::GpioStm32xxxx gpio_port_e_{reinterpret_cast<void*>(GPIOE_BASE), 4};
  gpio::GpioStm32xxxx gpio_port_h_{reinterpret_cast<void*>(GPIOH_BASE), 7};
  // Don't forget to modify NUM_GPIO_PORTS and add a GPIO_PORT_* above.

  power::PowerStm32L4xx power_{};

  dac::DacStm32xxxx<NUM_DAC_CHANNELS> dac_{DAC};

  dma::DmaStm32l4xx dma1_{DMA1};
  dma::DmaStm32l4xx dma2_{DMA2};

  adc::AdcStm32l4xx adc_{ADC1, dma1_, DMA1_Channel1, DMA_REQUEST_0};

  timer::TimerStm32l4xx timer2_{Stm32PeripheralIds::TIM2_ID, TIM2};

  rcc::LsiOscillatorStm32L4xx lsi_oscillator_{};
  timer::Stm32l4xxLptim lptim1_{Stm32PeripheralIds::LPTIM1_ID, LPTIM1, lsi_oscillator_};

  rcc::Hsi48OscillatorStm32L4xx hsi48_oscillator_{};
  random::RngStm32xxxx rng_{hsi48_oscillator_};

  watchdog::WatchdogStm32l4xx iwdg_{IWDG, lsi_oscillator_};

  i2c::I2cStm32l4xx i2c1_{I2C1, gpio_port_b_, /* SCL Pin */ 6, /* SDA Pin */ 7};
  i2c::I2cStm32l4xx i2c2_{I2C2, gpio_port_b_, /* SCL Pin */ 10, /* SDA Pin */ 11};
  i2c::I2cStm32l4xx i2c3_{I2C3, gpio_port_c_, /* SCL Pin */ 0, /* SDA Pin */ 1};

  can_bus::CanBusStm32l4xx can1_{CAN1,
                                 gpio_port_a_,
                                 /* TX Pin */ 12,
                                 /* RX Pin */ 11,
                                 /* SHUTDOWN Pin */ 9,
                                 /* SILENT Pin */ 10};

  imu::Bmi323Imu imu1_{0x68, i2c1_};
  imu::Bmi323Imu imu2_{0x69, i2c2_};

  power_monitor::Ina260PowerMonitor power_monitor1_{0x40, i2c3_};
  power_monitor::Ina260PowerMonitor power_monitor2_{0x41, i2c3_};

  programmer::ProgrammerStm32l4xx programmer_{gpio_port_b_,                //
                                              /* SWDIO_CONTROL Pin */ 15,  //
                                              /* SWCLK_CONTROL Pin */ 13,  //
                                              /* NRST_CONTROL Pin */ 14};

  mcu::McuStm32l4xx mcu_{};

  // Note that these GPIO Ports are disallowed on this board. They are marked private to make it
  // more difficult to accidentally use them.
  static constexpr gpio::PortNumber GPIO_PORT_F{5};
  static constexpr gpio::PortNumber GPIO_PORT_G{6};
  static constexpr size_t NUM_DISALLOWED_PORTS{2};

  static Board board_;

  // Private constructor to restrict inadvertent instantiation and copying.
  Board();

 public:
  // One board per executable.
  static Board& board();

  template <gpio::PortNumber GPIO_PORT>
  gpio::GpioPeripheral& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS + NUM_DISALLOWED_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
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
    if constexpr (GPIO_PORT == 3) {
      return gpio_port_d_;
    }
    if constexpr (GPIO_PORT == 4) {
      return gpio_port_e_;
    }
    if constexpr (GPIO_PORT == 7) {
      return gpio_port_h_;
    }
  }

  /**
   * Accessor for GPIO periperhals. Note that the templated version above is vastly preferred, as it
   * gives compile-time errors, as opposed to runtime errors like this method. This method should
   * only be used as to lookup GPIO ports using integer values that can be evaluated at
   * compile-time; these integer values will mainly come from STM's HAL.
   */
  gpio::GpioPeripheral& gpio(gpio::PortNumber port) {
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
    } else if (port == 7) {
      return gpio_port_h_;
    }
    error();
  }

  rcc::Rcc& rcc() { return rcc_; };

  power::Power& power() { return power_; }

  dac::DacPeripheral& dac() { return dac_; }

  adc::AdcPeripheral& adc() { return adc_; }

  timer::TimerPeripheral& timer2() { return timer2_; }
  timer::TimerPeripheral& sleep_timer() { return lptim1_; }

  systick::SysTickType& sys_tick() { return sys_tick_; }

  random::RngPeripheral& rng() { return rng_; }

  gpio::PinPeripheral debug_led() {
    return gpio::PinPeripheral{gpio<DEBUG_LED_PORT>(), DEBUG_LED_PIN};
  }

  gpio::PinPeripheral debug_led(size_t led_number) {
    return gpio::PinPeripheral{gpio(DEBUG_LED_PORTS[led_number]), DEBUG_LED_PINS[led_number]};
  }

  watchdog::WatchdogPeripheral& iwdg() { return iwdg_; }

  i2c::I2cPeripheral& i2c1() { return i2c1_; }
  i2c::I2cPeripheral& i2c2() { return i2c2_; }
  i2c::I2cPeripheral& i2c3() { return i2c3_; }

  can_bus::CanBusPeripheral& can1() { return can1_; }

  imu::ImuPeripheral& imu1() { return imu1_; }
  imu::ImuPeripheral& imu2() { return imu2_; }

  power_monitor::PowerMonitorPeripheral& power_monitor1() { return power_monitor1_; }
  power_monitor::PowerMonitorPeripheral& power_monitor2() { return power_monitor2_; }

  programmer::ProgrammerPeripheral& programmer() { return programmer_; }

  mcu::Mcu& mcu() { return mcu_; }
};

}  // namespace tvsc::hal::board
