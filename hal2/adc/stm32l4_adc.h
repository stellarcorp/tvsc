#pragma once

#include <chrono>
#include <cstdint>

#include "error/except.h"
#include "hal2/adc/adc.h"
#include "hal2/dma/dma.h"
#include "hal2/dma/stm32l4_dma.h"
#include "hal2/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::adc {

class Stm32L4Adc final : public AdcDevice {
 private:
  ADC_HandleTypeDef adc_{};
  dma::DmaController* dma_controller_;
  dma::DmaChannel dma_channel_{};
  const uint8_t adc_index_;
  const uint8_t dma_channel_index_;

  void enable() noexcept override;
  void disable() noexcept override;

  void start(const AdcSessionConfiguration&) noexcept override;
  [[nodiscard]] bool is_running() noexcept override;
  void stop() noexcept override;

  [[nodiscard]] uint16_t measure_value(
      gpio::PinRef pin, AdcSessionConfiguration::SampleCycles sample_time =
                            AdcSessionConfiguration::DEFAULT_SAMPLE_CYCLES) noexcept override;

  void calibrate_single_ended_input() noexcept override;
  void calibrate_differential_input() noexcept override;
  [[nodiscard]] uint32_t read_calibration_factor() noexcept override;
  void write_calibration_factor(uint32_t factor) noexcept override;

 public:
  Stm32L4Adc(uint8_t adc_index, dma::DmaController& dma_controller, uint8_t dma_channel_index)
      : dma_controller_(&dma_controller),
        adc_index_(adc_index),
        dma_channel_index_(dma_channel_index) {
    if (adc_index_ == 0) {
      adc_.Instance = ADC1;
#ifdef ADC2
    } else if (adc_index_ == 1) {
      adc_.Instance = ADC2;
#endif
#ifdef ADC3
    } else if (adc_index_ == 2) {
      adc_.Instance = ADC3;
#endif
    } else {
      except<std::logic_error>("Requested adc_index not defined in HAL.");
    }
  }
};

}  // namespace tvsc::hal2::adc
