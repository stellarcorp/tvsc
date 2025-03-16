#pragma once

#include "hal/dma/dma.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dma {

class DmaStm32l4xx final : public DmaPeripheral {
 private:
  DMA_TypeDef* dma_controller_;

  void enable() override;
  void disable() override;

 public:
  DmaStm32l4xx(DMA_TypeDef* dma_controller) : dma_controller_(dma_controller) {}
};

}  // namespace tvsc::hal::dma
