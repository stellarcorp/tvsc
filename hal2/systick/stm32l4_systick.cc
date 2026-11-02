#include "hal2/systick/stm32l4_systick.h"

#include "hal2/systick/stm32l4_systick_config.h"
#include "hal2/systick/systick.h"
#include "hal2/time_type.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::systick {

Stm32L4SysTick::Stm32L4SysTick() noexcept : previous_control_value_(SysTick->CTRL) {
  HAL_NVIC_SetPriority(SysTick_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(SysTick_IRQn);
}

[[nodiscard]] TimeType Stm32L4SysTick::current_time_micros() const noexcept { return uwTick; }

void Stm32L4SysTick::increment_micros(TimeType us) noexcept { uwTick += us; }

void Stm32L4SysTick::mask_interrupt() noexcept {
  previous_control_value_ = SysTick->CTRL;
  SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
}

void Stm32L4SysTick::unmask_interrupt() noexcept { SysTick->CTRL = previous_control_value_; }

void handle_systick_interrupt() noexcept {
  // SysTick fires every millisecond. We keep the tick counter in microseconds in order to better
  // avoid round-off errors.
  uwTick += 1000;
}

}  // namespace tvsc::hal2::systick
