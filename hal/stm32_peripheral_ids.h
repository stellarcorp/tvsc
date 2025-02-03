#pragma once

#include "hal/peripheral_id.h"

namespace tvsc::hal {

/**
 * Numerical ids identifying each of the peripherals in STM32 MCUs. The symbols in the enum attempt
 * to use the same names as in the code and literature from ST Micro. This list is not exhaustive;
 * we add ids when they are needed. Also, not all peripherals identified here are available in every
 * board. We could make enums for every board. That would allow us to catch more errors at
 * compile-time. But, so far, it hasn't been worth the extra design and maintenance effort.
 */
class Stm32PeripheralIds final {
 public:
  // Timers
  static constexpr PeripheralId TIM1{1};
  static constexpr PeripheralId TIM2{2};
  static constexpr PeripheralId TIM3{3};
  static constexpr PeripheralId TIM15{4};
  static constexpr PeripheralId LPTIM1{5};

  // RCC

  // GPIOs

  // I2C buses

  // SPI buses

  // CAN buses

  // DMAs

  // ADCs

  // DACs

  // RNGs

  // Cryptography modules
};

}  // namespace tvsc::hal
