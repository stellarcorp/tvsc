#pragma once

#include <cstdint>
#include <new>

#include "hal/adc/adc.h"
#include "hal/adc/stm32h7xx_adc_register_bank.h"
#include "hal/gpio/gpio.h"

namespace tvsc::hal::adc {

class AdcStm32h7xx final : public Adc {
 private:
  Stm32h7xxAdcRegisterBank* const registers_;

 public:
  AdcStm32h7xx(void* base_address) : registers_(new (base_address) Stm32h7xxAdcRegisterBank) {}

  void measure(gpio::Port port, gpio::Pin pin, uint8_t gain = 1) override;
  uint16_t read_result() override;

  bool is_running() override;
  void stop() override;
};

}  // namespace tvsc::hal::adc
