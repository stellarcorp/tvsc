#include "hal2/dma/stm32l4_dma.h"

#include <stdexcept>

#include "error/except.h"
#include "hal2/devices/device.h"
#include "hal2/dma/dma.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::dma {

namespace {

constexpr uint32_t translate_data_width(DmaSessionConfiguration::TransferWidth width) noexcept {
  switch (width) {
    case DmaSessionConfiguration::TransferWidth::BYTE:
      return DMA_PDATAALIGN_BYTE;
    case DmaSessionConfiguration::TransferWidth::HALF_WORD:
      return DMA_PDATAALIGN_HALFWORD;
    case DmaSessionConfiguration::TransferWidth::WORD:
      return DMA_PDATAALIGN_WORD;
  }
  __builtin_unreachable();
}

constexpr uint32_t translate_direction(DmaSessionConfiguration::Direction direction) noexcept {
  switch (direction) {
    case DmaSessionConfiguration::Direction::FROM_PERIPHERAL:
      return DMA_PERIPH_TO_MEMORY;
    case DmaSessionConfiguration::Direction::TO_PERIPHERAL:
      return DMA_MEMORY_TO_PERIPH;
  }
  __builtin_unreachable();
}

constexpr uint32_t translate_stream_mode(DmaSessionConfiguration::StreamMode mode) noexcept {
  switch (mode) {
    case DmaSessionConfiguration::StreamMode::SINGLE_SHOT:
      return DMA_NORMAL;
    case DmaSessionConfiguration::StreamMode::CONTINUOUS:
      return DMA_CIRCULAR;
  }
  __builtin_unreachable();
}

constexpr uint32_t translate_priority(DmaSessionConfiguration::Priority priority) noexcept {
  switch (priority) {
    case DmaSessionConfiguration::Priority::LOW:
      return DMA_PRIORITY_LOW;
    case DmaSessionConfiguration::Priority::MEDIUM:
      return DMA_PRIORITY_MEDIUM;
    case DmaSessionConfiguration::Priority::HIGH:
      return DMA_PRIORITY_HIGH;
    case DmaSessionConfiguration::Priority::VITAL:
      return DMA_PRIORITY_VERY_HIGH;
  }
  __builtin_unreachable();
}

}  // namespace

Stm32L4DmaController::Stm32L4DmaController(uint8_t controller_index) noexcept
    : controller_index_(controller_index) {
  // Statically bind each handle instance to its corresponding physical STM32L4 channel.
  for (size_t i = 0; i < NUM_CHANNELS; ++i) {
    auto& hdma = hdma_handles_[i];

    if (controller_index_ == 0) {
      switch (i) {
        case 0:
          hdma.Instance = DMA1_Channel1;
          break;
        case 1:
          hdma.Instance = DMA1_Channel2;
          break;
        case 2:
          hdma.Instance = DMA1_Channel3;
          break;
        case 3:
          hdma.Instance = DMA1_Channel4;
          break;
        case 4:
          hdma.Instance = DMA1_Channel5;
          break;
        case 5:
          hdma.Instance = DMA1_Channel6;
          break;
        case 6:
          hdma.Instance = DMA1_Channel7;
          break;
        default:
          __builtin_unreachable();
      }
    } else if (controller_index_ == 1) {
      switch (i) {
        case 0:
          hdma.Instance = DMA2_Channel1;
          break;
        case 1:
          hdma.Instance = DMA2_Channel2;
          break;
        case 2:
          hdma.Instance = DMA2_Channel3;
          break;
        case 3:
          hdma.Instance = DMA2_Channel4;
          break;
        case 4:
          hdma.Instance = DMA2_Channel5;
          break;
        case 5:
          hdma.Instance = DMA2_Channel6;
          break;
        case 6:
          hdma.Instance = DMA2_Channel7;
          break;
        default:
          __builtin_unreachable();
      }
    } else {
      __builtin_unreachable();
    }
  }
}

void Stm32L4DmaController::enable() noexcept {
  if (controller_index_ == 0) {
    __HAL_RCC_DMA1_CLK_ENABLE();
  } else if (controller_index_ == 1) {
    __HAL_RCC_DMA2_CLK_ENABLE();
  } else {
    __builtin_unreachable();
  }
}

void Stm32L4DmaController::disable() noexcept {
  if (controller_index_ == 0) {
    __HAL_RCC_DMA1_CLK_DISABLE();
  } else if (controller_index_ == 1) {
    __HAL_RCC_DMA2_CLK_DISABLE();
  } else {
    __builtin_unreachable();
  }
}

void Stm32L4DmaController::release_channel(uint8_t channel_index) noexcept {
  channel_in_use_.at(channel_index) = false;
}

DmaChannel Stm32L4DmaController::acquire_channel(uint8_t channel_index) noexcept {
  if (channel_in_use_.at(channel_index)) {
    except<std::logic_error>("Attempt to acquire DMA channel already in use.");
  }

  channel_in_use_.at(channel_index) = true;
  return DmaChannel{*this, channel_index};
}

bool Stm32L4DmaController::in_use(uint8_t channel_index) noexcept {
  return channel_in_use_.at(channel_index);
}

void Stm32L4DmaController::configure(uint8_t channel_index,
                                     const DmaSessionConfiguration& config) noexcept {
  auto* hdma = &hdma_handles_.at(channel_index);

  hdma->Init.Direction = translate_direction(config.direction);
  hdma->Init.PeriphInc = DMA_PINC_DISABLE;
  hdma->Init.MemInc = DMA_MINC_ENABLE;
  hdma->Init.PeriphDataAlignment = translate_data_width(config.peripheral_data_width);
  hdma->Init.MemDataAlignment = translate_data_width(config.memory_data_width);
  hdma->Init.Mode = translate_stream_mode(config.stream_mode);
  hdma->Init.Priority = translate_priority(config.priority);

  HAL_DMA_Init(hdma);
}

void* Stm32L4DmaController::implementation_handle(uint8_t channel_index) noexcept {
  return static_cast<void*>(&hdma_handles_.at(channel_index));
}

const void* Stm32L4DmaController::implementation_handle(uint8_t channel_index) const noexcept {
  return static_cast<const void*>(&hdma_handles_.at(channel_index));
}

}  // namespace tvsc::hal2::dma
