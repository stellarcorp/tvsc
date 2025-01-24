#pragma once

#include "hal/register.h"

namespace tvsc::hal::adc {

class Stm32h7xxAdcRegisterBank final {
 public:
  // Offset 0x00
  volatile Register ISR;

  // Offset 0x04
  volatile Register IER;

  // Register with bit fields to start and stop the ADC. Includes other functions as well.
  // Offset 0x08
  volatile Register CR;

  // Configuration registers with many different parameters.
  // Offset 0x0c
  volatile Register CFGR;
  // Offset 0x10
  volatile Register CFGR2;

  // Sample rate configuration for channels 0-9.
  // Offset 0x14
  volatile Register SMPR1;

  // Sample rate configuration for channels 10-18.
  // Offset 0x18
  volatile Register SMPR2;

  std::byte unused2[0x30 - 0x18 - sizeof(Register)];

  // Offset 0x30
  volatile Register SQR1;

  std::byte unused3[0x40 - 0x30 - sizeof(Register)];

  // Offset 0x40
  volatile Register DR;
};

}  // namespace tvsc::hal::adc
