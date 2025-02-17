#pragma once

#include "hal/dma/dma.h"
#include "hal/peripheral_id.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dma {

class DmaStm32l4xx final : public DmaPeripheral {
 private:
  DMA_HandleTypeDef dma_{};
  PeripheralId id_;

  PeripheralId id() override;

  void start_circular_transfer() override;

  void handle_interrupt() override;

  void enable() override;
  void disable() override;

 public:
  DmaStm32l4xx(PeripheralId id, DMA_Channel_TypeDef* dma_instance, uint32_t request_mapping)
      : id_(id) {
    dma_.Instance = dma_instance;
    dma_.Init.Request = request_mapping;
  }

  // Expose the raw handle. DMA requests on the STM32L4xx link the parent handle (ADC, DAC, etc.) to
  // the DMA handle in order to manage the DMA as part of that process.
  DMA_HandleTypeDef* handle() { return &dma_; }
};

}  // namespace tvsc::hal::dma
