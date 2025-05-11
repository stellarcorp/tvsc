#pragma once

#include <chrono>
#include <cstdint>

#include "hal/adc/adc.h"
#include "hal/dma/dma.h"
#include "hal/dma/stm32l4xx_dma.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::adc {

class AdcStm32l4xx final : public AdcPeripheral {
 private:
  ADC_HandleTypeDef adc_{};
  ADC_ChannelConfTypeDef channel_config_{};
  DMA_HandleTypeDef dma_handle_{};
  dma::DmaStm32l4xx* dma_peripheral_;
  dma::Dma dma_{};

  void enable() override;
  void disable() override;

  void start_single_conversion(gpio::PortPin pin, uint32_t* destination,
                               size_t destination_buffer_size) override;

  void start_conversion_stream(gpio::PortPin pin, uint32_t* destination,
                               size_t destination_buffer_size, timer::Timer& trigger) override;

  uint16_t measure_value(gpio::PortPin pin, std::chrono::milliseconds timeout) override;

  void reset_after_conversion() override;

  void set_resolution(uint8_t bits_resolution) override;

  void use_data_align_left() override;
  void use_data_align_right() override;

  void calibrate_single_ended_input() override;
  void calibrate_differential_input() override;
  uint32_t read_calibration_factor() override;
  void write_calibration_factor(uint32_t factor) override;

  bool is_running() override;
  void stop() override;

 public:
  AdcStm32l4xx(ADC_TypeDef* adc_instance, dma::DmaStm32l4xx& dma_peripheral,
               DMA_Channel_TypeDef* channel, uint32_t dma_request)
      : dma_peripheral_(&dma_peripheral) {
    adc_.Instance = adc_instance;
    adc_.Init.Resolution = ADC_RESOLUTION_12B;
    adc_.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    dma_handle_.Instance = channel;
    dma_handle_.Init.Request = dma_request;
  }

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::adc
