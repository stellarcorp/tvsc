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

  void start_conversion(gpio::Port port, gpio::Pin pin, uint8_t gain = 1) override;
  uint16_t read_result() override;

  void calibrate_single_ended_input() override;
  void calibrate_differential_input() override;
  uint16_t read_calibration_factor() override;
  void write_calibration_factor(uint16_t factor) override;

  bool is_running() override;
  void stop() override;
};

}  // namespace tvsc::hal::adc
