#pragma once

#include "hal2/systick/systick.h"
#include "hal2/time_type.h"

namespace tvsc::hal2::systick {

class Stm32L4SysTick final : public SysTickType {
 private:
  uint32_t previous_control_value_;

 public:
  Stm32L4SysTick() noexcept;
  [[nodiscard]] TimeType current_time_micros() const noexcept override;
  void increment_micros(TimeType us) noexcept override;

  void mask_interrupt() noexcept override;
  void unmask_interrupt() noexcept override;
};

extern void handle_systick_interrupt() noexcept;

}  // namespace tvsc::hal2::systick
