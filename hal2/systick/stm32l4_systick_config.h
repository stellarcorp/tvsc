#pragma once

#include "hal2/config/facility.h"
#include "hal2/config/system_builder.h"
#include "hal2/config/vector_table.h"
#include "hal2/systick/stm32l4xx_systick.h"
#include "hal2/systick/systick.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

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
