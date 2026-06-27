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
#include "hal/mcu_identification/mcu_identification.h"
#include "hal/mcu_identification/stm32l4xx_mcu_identification.h"
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

template <size_t NUMBER_GPIO_PORTS, size_t NUMBER_I2C_BUSES, size_t NUMBER_CAN_BUSES,
          size_t NUMBER_DAC_CHANNELS, size_t NUMBER_ADC_CHANNELS>
class McuStm32L4xx final {
 public:
  static constexpr size_t NUM_GPIO_PORTS{NUMBER_GPIO_PORTS};
  static constexpr size_t NUM_I2C_BUSES{NUMBER_I2C_BUSES};
  static constexpr size_t NUM_CAN_BUSES{NUMBER_CAN_BUSES};
  static constexpr size_t NUM_DAC_CHANNELS{NUMBER_DAC_CHANNELS};
  static constexpr size_t NUM_ADC_CHANNELS{NUMBER_ADC_CHANNELS};

  static constexpr gpio::PortNumber GPIO_PORT_A{0};
  static constexpr gpio::PortNumber GPIO_PORT_B{1};
  static constexpr gpio::PortNumber GPIO_PORT_C{2};
  static constexpr gpio::PortNumber GPIO_PORT_D{3};
  static constexpr gpio::PortNumber GPIO_PORT_E{4};
  static constexpr gpio::PortNumber GPIO_PORT_F{5};
  static constexpr gpio::PortNumber GPIO_PORT_G{6};
  static constexpr gpio::PortNumber GPIO_PORT_H{7};

  static constexpr gpio::PortNumber DAC_CHANNEL_1_PORT{GPIO_PORT_A};
  static constexpr gpio::PinNumber DAC_CHANNEL_1_PIN{4};

  static constexpr std::array<gpio::PortNumber, NUM_DAC_CHANNELS> DAC_PORTS{DAC_CHANNEL_1_PORT};
  static constexpr std::array<gpio::PinNumber, NUM_DAC_CHANNELS> DAC_PINS{DAC_CHANNEL_1_PIN};

 private:
  rcc::RccStm32L4xx rcc_{};

  systick::SysTickStm32l4xx sys_tick_{};
  /*
      std::array<gpio::GpioStm32xxxx, NUM_GPIO_PORTS> gpio_ports_{
  #if defined(GPIOA_BASE) && (NUM_GPIO_PORTS > 1)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOA_BASE), GPIO_PORT_A},
  #endif
  #if defined(GPIOB_BASE) && (NUM_GPIO_PORTS > 2)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOB_BASE), GPIO_PORT_B},
  #endif
  #if defined(GPIOC_BASE) && (NUM_GPIO_PORTS > 3)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOC_BASE), GPIO_PORT_C},
  #endif
  #if defined(GPIOD_BASE) && (NUM_GPIO_PORTS > 4)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOD_BASE), GPIO_PORT_D},
  #endif
  #if defined(GPIOE_BASE) && (NUM_GPIO_PORTS > 5)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOE_BASE), GPIO_PORT_E},
  #endif
  #if defined(GPIOF_BASE) && (NUM_GPIO_PORTS > 6)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOF_BASE), GPIO_PORT_F},
  #endif
  #if defined(GPIOG_BASE) && (NUM_GPIO_PORTS > 7)
        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOG_BASE), GPIO_PORT_G},
  #endif

        gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOH_BASE), GPIO_PORT_H},
    };
  */
  std::array<gpio::GpioStm32xxxx, NUM_GPIO_PORTS> gpio_ports_{
#if defined(GPIOA_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOA_BASE), GPIO_PORT_A},
#endif
#if defined(GPIOB_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOB_BASE), GPIO_PORT_B},
#endif
#if defined(GPIOC_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOC_BASE), GPIO_PORT_C},
#endif
#if defined(GPIOD_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOD_BASE), GPIO_PORT_D},
#endif
#if defined(GPIOE_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOE_BASE), GPIO_PORT_E},
#endif
#if defined(GPIOF_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOF_BASE), GPIO_PORT_F},
#endif
#if defined(GPIOG_BASE)
      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOG_BASE), GPIO_PORT_G},
#endif

      gpio::GpioStm32xxxx{reinterpret_cast<void*>(GPIOH_BASE), GPIO_PORT_H},
  };

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

  std::array<i2c::I2cStm32l4xx, NUM_I2C_BUSES> i2c_buses{
      i2c::I2cStm32l4xx{I2C1, gpio<GPIO_PORT_B>(), /* SCL Pin */ 6, /* SDA Pin */ 7},
      i2c::I2cStm32l4xx{I2C2, gpio<GPIO_PORT_B>(), /* SCL Pin */ 10, /* SDA Pin */ 11},
      i2c::I2cStm32l4xx{I2C3, gpio<GPIO_PORT_C>(), /* SCL Pin */ 0, /* SDA Pin */ 1},
  };

  std::array<can_bus::CanBusStm32l4xx, NUM_CAN_BUSES> can_buses{
      can_bus::CanBusStm32l4xx{CAN1, gpio<GPIO_PORT_A>(),
                               /* TX Pin */ 12,
                               /* RX Pin */ 11,
                               /* SHUTDOWN Pin */ 9,
                               /* SILENT Pin */ 10},
  };

  mcu_identification::McuIdentificationStm32l4xx mcu_identification_{};

  // Note that these GPIO Ports are disallowed on this board. They are marked private to make it
  // more difficult to accidentally use them.
  static constexpr size_t NUM_DISALLOWED_PORTS{2};

  // Private constructor to restrict inadvertent instantiation and copying.
  McuStm32L4xx() { internal::configure_interrupts(); }

 public:
  // One MCU per executable.
  static McuStm32L4xx& mcu() {
    static McuStm32L4xx mcu_{};
    return mcu_;
  }

  template <gpio::PortNumber GPIO_PORT>
  gpio::GpioPeripheral& gpio() {
    static_assert((GPIO_PORT < NUM_GPIO_PORTS - 1) or (GPIO_PORT == GPIO_PORT_H),
                  "Invalid GPIO port id. Likely, there is a mismatch in the build that "
                  "instantiates the wrong MCU or attempts to use the wrong MCU. Verify that the "
                  "board-specific header file (hal/mcu/<mcu_type>.h) is being included.");
    if constexpr (GPIO_PORT == GPIO_PORT_H) {
      return gpio_ports_[NUM_GPIO_PORTS - 1];
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
  gpio::GpioPeripheral& gpio(gpio::PortNumber port) {
    if (port == GPIO_PORT_H) {
      return gpio_ports_[NUM_GPIO_PORTS - 1];
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
    static_assert(BUS < NUM_I2C_BUSES);
    return i2c_buses[BUS];
  }

  template <size_t BUS = 0>
  can_bus::CanBusPeripheral& can() noexcept {
    static_assert(BUS < NUM_CAN_BUSES);
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
using Mcu = McuStm32L4xx</* GPIO ports */ 6, /* I2C buses, actual max is 4, but 3 are common */ 3,
                         /* CAN buses */ 1, /* DACs */ 1,
                         /* ADCs */ 1>;
#endif

}  // namespace tvsc::hal::mcu
