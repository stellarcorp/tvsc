#pragma once

#include "hal2/config/facility.h"
#include "hal2/config/system_builder.h"
#include "hal2/config/vector_table.h"
#include "hal2/systick/stm32l4xx_systick.h"
#include "hal2/systick/systick.h"

namespace tvsc::hal2::systick {

class Stm32L4SysTickAccessor final {
 private:
  SysTickStm32l4xx systick_{};

 public:
  [[nodiscard]] SysTickType& get() noexcept { return systick_; }
  [[nodiscard]] SysTickType& systick() noexcept { return get(); }
};

struct Stm32L4SysTickConfiguration final {
  using accessor = Stm32L4SysTickAccessor;

  static constexpr auto facility() noexcept { return config::Facility::SYSTICK; }
  static constexpr size_t index() noexcept { return 0; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(SysTick_IRQn, handle_systick_interrupt);
  }
};
static_assert(ValidPeripheralConfiguration<Stm32L4SysTickConfiguration>);

}  // namespace tvsc::hal2::systick
