#pragma once

#include <cstdint>
#include <new>

#include "hal/adc/adc.h"
#include "hal/dma/dma.h"
#include "hal/dma/stm32l4xx_dma.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::adc {

class AdcStm32l4xx final : public Adc {
 private:
  ADC_HandleTypeDef adc_{};
  dma::DmaStm32l4xx* dma_;
  ADC_ChannelConfTypeDef channel_config_{};

 public:
  AdcStm32l4xx(ADC_TypeDef* adc_instance, dma::DmaStm32l4xx& dma) : dma_(&dma) {
    adc_.Instance = adc_instance;
  }

  void start_conversion(gpio::PortPin pin, uint32_t* destination,
                        size_t destination_buffer_size) override;

  void calibrate_single_ended_input() override;
  void calibrate_differential_input() override;
  uint32_t read_calibration_factor() override;
  void write_calibration_factor(uint32_t factor) override;

  bool is_running() override;
  void stop() override;

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::adc
