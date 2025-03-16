#include "hal/dma/stm32l4xx_dma.h"

#include "hal/error.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dma {

void DmaStm32l4xx::enable() {
  if (dma_controller_ == DMA1) {
    __HAL_RCC_DMA1_CLK_ENABLE();
  } else if (dma_controller_ == DMA2) {
    __HAL_RCC_DMA2_CLK_ENABLE();
  } else {
    error();
  }
}

void DmaStm32l4xx::disable() {
  if (dma_controller_ == DMA1) {
    __HAL_RCC_DMA1_CLK_DISABLE();
  } else if (dma_controller_ == DMA2) {
    __HAL_RCC_DMA2_CLK_DISABLE();
  } else {
    error();
  }
}

}  // namespace tvsc::hal::dma
