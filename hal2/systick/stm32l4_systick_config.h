#pragma once

#include "hal2/systick/stm32l4_systick.h"
#include "hal2/systick/systick.h"

namespace tvsc::hal2::systick {

template <typename System>
class Stm32L4SysTickAccessor {
 private:
  Stm32L4SysTick systick_{};

 public:
  [[nodiscard]] SysTickType& get() noexcept { return systick_; }
  [[nodiscard]] SysTickType& systick() noexcept { return get(); }
};

}  // namespace tvsc::hal2::systick
