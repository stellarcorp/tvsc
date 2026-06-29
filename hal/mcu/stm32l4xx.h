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
#include "hal/mcu/basic_mcu.h"
#include "hal/mcu_identification/mcu_identification.h"
#include "hal/mcu_identification/stm32l4xx_mcu_identification.h"
#include "hal/pinout/pinout.h"
#include "hal/power/power.h"
#include "hal/power/stm32l4xx_power.h"
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

namespace tvsc::hal::mcu {

namespace internal {
void configure_interrupts();
}

template <size_t MAX_GPIO_PORTS, size_t MAX_I2C_BUSES, size_t MAX_CAN_BUSES,
          size_t MAX_DAC_CHANNELS, size_t MAX_ADC_CHANNELS>
class McuStm32L4xx final {
 public:
  using PinoutType = pinout::Pinout;

  static_assert(PinoutType::NUM_GPIO_PORTS <= MAX_GPIO_PORTS);
  static_assert(PinoutType::NUM_I2C_BUSES <= MAX_I2C_BUSES);
  static_assert(PinoutType::NUM_CAN_BUSES <= MAX_CAN_BUSES);
  static_assert(PinoutType::NUM_DAC_CHANNELS <= MAX_DAC_CHANNELS);
  static_assert(PinoutType::NUM_ADC_CHANNELS <= MAX_ADC_CHANNELS);

 private:
  rcc::RccStm32L4xx rcc_{};

  systick::SysTickStm32l4xx sys_tick_{};

  std::array<gpio::GpioStm32xxxx, MAX_GPIO_PORTS> gpio_ports_{
#if defined(GPIOA_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOA_BASE), PinoutType::GPIO_PORT_A},
#endif
#if defined(GPIOB_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOB_BASE), PinoutType::GPIO_PORT_B},
#endif
#if defined(GPIOC_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOC_BASE), PinoutType::GPIO_PORT_C},
#endif
#if defined(GPIOD_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOD_BASE), PinoutType::GPIO_PORT_D},
#endif
#if defined(GPIOE_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOE_BASE), PinoutType::GPIO_PORT_E},
#endif
#if defined(GPIOF_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOF_BASE), PinoutType::GPIO_PORT_F},
#endif
#if defined(GPIOG_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOG_BASE), PinoutType::GPIO_PORT_G},
#endif

      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOH_BASE), PinoutType::GPIO_PORT_H},
  };

  power::PowerStm32L4xx power_{};

  dac::DacStm32xxxx<PinoutType::NUM_DAC_CHANNELS> dac_{DAC};

  dma::DmaStm32l4xx dma1_{DMA1};
  dma::DmaStm32l4xx dma2_{DMA2};

  adc::AdcStm32l4xx adc_{ADC1, dma1_, DMA1_Channel1, DMA_REQUEST_0};

  timer::TimerStm32l4xx timer2_{Stm32PeripheralIds::TIM2_ID, TIM2};

  rcc::LsiOscillatorStm32L4xx lsi_oscillator_{};
  timer::Stm32l4xxLptim lptim1_{Stm32PeripheralIds::LPTIM1_ID, LPTIM1, lsi_oscillator_};

  rcc::Hsi48OscillatorStm32L4xx hsi48_oscillator_{};
  random::RngStm32xxxx rng_{hsi48_oscillator_};

  watchdog::WatchdogStm32l4xx iwdg_{IWDG, lsi_oscillator_};

  std::array<i2c::I2cStm32l4xx, PinoutType::NUM_I2C_BUSES> i2c_buses{
      i2c::I2cStm32l4xx{I2C1, create_peripheral(PinoutType::I2C1_SCL_PIN),
                        create_peripheral(PinoutType::I2C1_SDA_PIN)},
      i2c::I2cStm32l4xx{I2C2, create_peripheral(PinoutType::I2C2_SCL_PIN),
                        create_peripheral(PinoutType::I2C2_SDA_PIN)},
      i2c::I2cStm32l4xx{I2C3, create_peripheral(PinoutType::I2C3_SCL_PIN),
                        create_peripheral(PinoutType::I2C3_SDA_PIN)},
  };

  std::array<can_bus::CanBusStm32l4xx, PinoutType::NUM_CAN_BUSES> can_buses{
      can_bus::CanBusStm32l4xx{CAN1, create_peripheral(PinoutType::CAN1_TX_PIN),
                               create_peripheral(PinoutType::CAN1_RX_PIN),
                               create_peripheral(PinoutType::CAN1_SHUTDOWN_PIN),
                               create_peripheral(PinoutType::CAN1_SILENT_PIN)},
  };

  mcu_identification::McuIdentificationStm32l4xx mcu_identification_{};

  // Private constructor to restrict inadvertent instantiation and copying.
  McuStm32L4xx() { internal::configure_interrupts(); }

 public:
  // One MCU per executable.
  static McuStm32L4xx& mcu() {
    static McuStm32L4xx mcu_{};
    return mcu_;
  }

  constexpr gpio::PinPeripheral create_peripheral(gpio::PinRef ref) noexcept {
    return {gpio(ref.port), ref.pin};
  }

  template <gpio::PortNumber GPIO_PORT>
  constexpr gpio::GpioPeripheral& gpio() noexcept {
    static_assert(
        (GPIO_PORT < PinoutType::NUM_GPIO_PORTS - 1) or (GPIO_PORT == PinoutType::GPIO_PORT_H),
        "Invalid GPIO port id. Possible issues: Invalid Pinout configuration. Invalid "
        "Mcu initialization code. It is also possible that there is a mismatch in the build that "
        "instantiates the wrong MCU or attempts to use the wrong MCU. Verify that the "
        "board-specific header file (hal/mcu/<mcu_type>.h) is being included.");
    if constexpr (GPIO_PORT == PinoutType::GPIO_PORT_H) {
      return gpio_ports_[MAX_GPIO_PORTS - 1];
    } else {
      return gpio_ports_[GPIO_PORT];
    }
  }

  /**
   * Accessor for GPIO periperhals. Note that the templated version above is vastly preferred, as it
   * gives compile-time errors, as opposed to runtime errors like this method. This method should
   * only be used to lookup GPIO ports using integer values that cannot be evaluated at
   * compile-time; these integer values will mainly come from STM's HAL.
   */
  constexpr gpio::GpioPeripheral& gpio(gpio::PortNumber port) noexcept {
    if (port == PinoutType::GPIO_PORT_H) {
      return gpio_ports_[MAX_GPIO_PORTS - 1];
    } else {
      return gpio_ports_.at(port);
    }
    error();
  }

  rcc::Rcc& rcc() { return rcc_; };

  power::Power& power() { return power_; }

  mcu_identification::McuIdentification& mcu_identification() { return mcu_identification_; }

  dac::DacPeripheral& dac() { return dac_; }

  adc::AdcPeripheral& adc() { return adc_; }

  timer::TimerPeripheral& timer2() { return timer2_; }
  timer::TimerPeripheral& sleep_timer() { return lptim1_; }

  systick::SysTickType& sys_tick() { return sys_tick_; }

  random::RngPeripheral& rng() { return rng_; }

  watchdog::WatchdogPeripheral& iwdg() { return iwdg_; }

  template <size_t BUS = 0>
  i2c::I2cPeripheral& i2c() noexcept {
    static_assert(BUS < PinoutType::NUM_I2C_BUSES);
    return i2c_buses[BUS];
  }

  template <size_t BUS = 0>
  can_bus::CanBusPeripheral& can() noexcept {
    static_assert(BUS < PinoutType::NUM_CAN_BUSES);
    return can_buses[BUS];
  }
};

#if defined(STM32L412xx)
using Mcu = McuStm32L4xx</* GPIO ports */ 4, /* I2C buses */ 3, /* CAN buses */ 0, /* DACs */ 0,
                         /* ADCs */ 2>;
#elif defined(STM32L432xx)
using Mcu = McuStm32L4xx</* GPIO ports */ 3, /* I2C buses */ 2, /* CAN buses */ 1, /* DACs */ 2,
                         /* ADCs */ 1>;
#elif defined(STM32L452xx)
using Mcu = McuStm32L4xx</* GPIO ports */ 6, /* I2C buses */ 4, /* CAN buses */ 1, /* DACs */ 1,
                         /* ADCs */ 1>;
#endif

static_assert(BasicMcu<Mcu>);

}  // namespace tvsc::hal::mcu
