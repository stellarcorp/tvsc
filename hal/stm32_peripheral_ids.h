#pragma once

#include "hal/peripheral_id.h"

namespace tvsc::hal {

/**
 * Numerical ids identifying each of the peripherals in STM32 MCUs. The symbols in the enumeration
 * attempt to use the same names as in the code and literature from ST Micro. We add the suffix of
 * ID, since ST defines the corresponding symbol as a macro, defeating any namespacing. This list is
 * not exhaustive; we add ids when they are needed. Also, not all peripherals identified here are
 * available in every board. We could make enums for every board. That would allow us to catch more
 * errors at compile-time. But, so far, it hasn't been worth the extra design and maintenance
 * effort.
 */
class Stm32PeripheralIds final {
 public:
  // Timers
  static constexpr PeripheralId TIM1_ID{1};
  static constexpr PeripheralId TIM2_ID{2};
  static constexpr PeripheralId TIM4_ID{3};
  static constexpr PeripheralId TIM6_ID{4};
  static constexpr PeripheralId TIM15_ID{5};

  static constexpr PeripheralId LPTIM1_ID{6};
  static constexpr PeripheralId LPTIM2_ID{7};

  // DMAs
  // Note that DMAs are mostly identified by channel, rather than the peripheral itself. In ST
  // Micro's HAL, the main configuration structure DMA_Handle_TypeDef has an instance of the channel
  // (DMA_Channel_TypeDef). So, we follow the same pattern and give ids for the channels in each DMA
  // peripheral.
  static constexpr PeripheralId DMA1_CHANNEL1_ID{20};

  // RCC

  // GPIOs

  // I2C buses

  // SPI buses

  // CAN buses

  // ADCs

  // DACs

  // RNGs

  // Cryptography modules
};

}  // namespace tvsc::hal
