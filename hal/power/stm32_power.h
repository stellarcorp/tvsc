#pragma once

#include <new>

#include "hal/power/power.h"
#include "hal/register.h"

namespace tvsc::hal::power {

class PowerRegisterBank final {
 public:
  // Power control register managing various power modes, voltage scaling, etc.
  // Offset 0x00
  volatile Register CR1;

  std::byte unused1[0x14 - sizeof(Register)];

  // Power status register 2 handling peripheral voltage monitoring and the status of the low power
  // regulator.
  // Offset 0x14
  volatile Register SR2;
};

class PowerStm32xxxx final : public Power {
 private:
  PowerRegisterBank* registers_;

 public:
  PowerStm32xxxx(void* base_address) : registers_(new (base_address) PowerRegisterBank) {}

  void enter_low_power_run_mode() override;
  void exit_low_power_run_mode() override;
};

}  // namespace tvsc::hal::power
