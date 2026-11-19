#pragma once

#include "hal2/devices/device.h"
#include "hal2/dma/dma.h"
#include "hal2/dma/stm32l4_dma.h"

namespace tvsc::hal2::dma {

template <size_t INDEX>
class Stm32L4DmaAccessor;

template <>
class Stm32L4DmaAccessor<0> {
 private:
  Stm32L4DmaController controller_{0};

 public:
  [[nodiscard]] DmaController& get() noexcept { return controller_; }
  [[nodiscard]] auto& dma0() noexcept { return get(); }
};

template <>
class Stm32L4DmaAccessor<1> {
 private:
  Stm32L4DmaController controller_{1};

 public:
  [[nodiscard]] DmaController& get() noexcept { return controller_; }
  [[nodiscard]] auto& dma1() noexcept { return get(); }
};

}  // namespace tvsc::hal2::dma
