#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal2/device.h"
#include "hal2/dma/dma.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::dma {

class Stm32L4DmaController final : public DmaController {
 public:
  static constexpr size_t NUM_CHANNELS{7};

  std::array<DMA_HandleTypeDef, NUM_CHANNELS> hdma_handles_{};
  std::array<bool, NUM_CHANNELS> channel_in_use_{};
  uint8_t controller_index_;

  void enable() noexcept override;
  void disable() noexcept override;

  void release_channel(uint8_t channel_index) noexcept override;

  void configure(uint8_t channel_index, const DmaSessionConfiguration& config) noexcept override;
  [[nodiscard]] void* implementation_handle(uint8_t channel_index) noexcept override;
  [[nodiscard]] const void* implementation_handle(uint8_t channel_index) const noexcept override;

 public:
  explicit Stm32L4DmaController(uint8_t controller_index) noexcept;

  Stm32L4DmaController(const Stm32L4DmaController&) = delete;
  Stm32L4DmaController& operator=(const Stm32L4DmaController&) = delete;
  Stm32L4DmaController(Stm32L4DmaController&&) = delete;
  Stm32L4DmaController& operator=(Stm32L4DmaController&&) = delete;

  [[nodiscard]] DmaChannel acquire_channel(uint8_t channel_index) noexcept override;
  [[nodiscard]] bool in_use(uint8_t channel_index) noexcept override;
};

template <uint8_t CONTROLLER, uint8_t CHANNEL>
  requires(CONTROLLER < 2) and (CHANNEL < Stm32L4DmaController::NUM_CHANNELS)
void handle_interrupt() {
  // TODO(james): Implement.
}

}  // namespace tvsc::hal2::dma
