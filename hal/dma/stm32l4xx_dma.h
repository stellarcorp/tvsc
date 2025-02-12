#pragma once

#include "hal/dma/dma.h"
#include "hal/enable_lock.h"
#include "hal/peripheral_id.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dma {

class DmaStm32l4xx final : public Dma {
 private:
  DMA_HandleTypeDef dma_{};
  uint32_t enable_counter_{0};
  PeripheralId id_;

 public:
  DmaStm32l4xx(PeripheralId id, DMA_Channel_TypeDef* dma_instance, uint32_t request_mapping)
      : id_(id) {
    dma_.Instance = dma_instance;
    dma_.Init.Request = request_mapping;
  }

  PeripheralId id() override;

  void start_circular_transfer() override;

  void handle_interrupt() override;

  // Expose the raw handle. DMA requests on the STM32L4xx link the parent handle (ADC, DAC, etc.) to
  // the DMA handle in order to manage the DMA as part of that process.
  DMA_HandleTypeDef* handle() { return &dma_; }

  EnableLock enable() override;
};

}  // namespace tvsc::hal::dma
