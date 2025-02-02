#include "hal/dma/stm32l4xx_dma.h"

#include "hal/power_token.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::dma {

void DmaStm32l4xx::handle_interrupt() { HAL_DMA_IRQHandler(&dma_); }

void DmaStm32l4xx::start_circular_transfer() {
  // Configure DMA
  dma_.Init.Direction = DMA_PERIPH_TO_MEMORY;
  dma_.Init.PeriphInc = DMA_PINC_DISABLE;
  dma_.Init.MemInc = DMA_MINC_ENABLE;
  // TODO(james): These alignment values are suspect.
  dma_.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  dma_.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  dma_.Init.Mode = DMA_CIRCULAR;
  dma_.Init.Priority = DMA_PRIORITY_HIGH;

  HAL_DMA_Init(&dma_);
}

void turn_off() { __HAL_RCC_DMA1_CLK_DISABLE(); }

PowerToken DmaStm32l4xx::turn_on() {
  if (use_counter_ == 0) {
    __HAL_RCC_DMA1_CLK_ENABLE();
  }
  ++use_counter_;
  return PowerToken([this]() {
    --use_counter_;
    if (use_counter_ == 0) {
      turn_off();
    }
  });
}

}  // namespace tvsc::hal::dma
