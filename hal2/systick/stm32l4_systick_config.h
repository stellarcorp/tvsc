#pragma once

#include "hal2/systick/stm32l4xx_systick.h"
#include "hal2/systick/systick.h"

namespace tvsc::hal2::systick {

template <typename System>
class Stm32L4SysTickAccessor {
 private:
  SysTickStm32l4xx systick_{};

 public:
  [[nodiscard]] SysTickType& get() noexcept { return systick_; }
  [[nodiscard]] SysTickType& systick() noexcept { return get(); }
};

}  // namespace tvsc::hal2::systick
