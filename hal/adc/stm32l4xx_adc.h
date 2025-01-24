#pragma once

#include <cstdint>
#include <new>

#include "hal/adc/adc.h"
#include "hal/adc/stm32l4xx_adc_register_bank.h"
#include "hal/gpio/gpio.h"

namespace tvsc::hal::adc {

class AdcStm32L4xx final : public Adc {
 private:
  Stm32l4xxAdcRegisterBank* const registers_;

 public:
  AdcStm32L4xx(void* base_address) : registers_(new (base_address) Stm32l4xxAdcRegisterBank) {}

  void measure(gpio::Port port, gpio::Pin pin, uint8_t gain = 1) override;
  uint16_t read_result() override;

  bool is_running() override;
  void stop() override;
};

}  // namespace tvsc::hal::adc
