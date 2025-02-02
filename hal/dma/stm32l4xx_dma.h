#pragma once

#include "hal/dma/dma.h"
#include "hal/power_token.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dma {

class DmaStm32l4xx final : public Dma {
 private:
  DMA_HandleTypeDef dma_{};
  uint16_t use_counter_{0};

 public:
  DmaStm32l4xx(DMA_Channel_TypeDef* dma_instance, uint32_t request_mapping) {
    dma_.Instance = dma_instance;
    dma_.Init.Request = request_mapping;
  }

  void start_circular_transfer() override;

  void handle_interrupt() override;

  // Expose the raw handle. DMA requests on the STM32L4xx link the parent handle (ADC, DAC, etc.) to
  // the DMA handle in order to manage the DMA as part of that process.
  DMA_HandleTypeDef* handle() { return &dma_; }

  PowerToken turn_on() override;
};

}  // namespace tvsc::hal::dma
