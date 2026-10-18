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

struct Stm32L4SysTickConfiguration {
  template <typename System>
  using accessor = Stm32L4SysTickAccessor<System>;

  static constexpr auto facility() noexcept { return config::Facility::SYSTICK_FACILITY; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(SysTick_IRQn, handle_systick_interrupt);
  }
};
static_assert(config::ValidPeripheralConfiguration<Stm32L4SysTickConfiguration>);

}  // namespace tvsc::hal2::systick
