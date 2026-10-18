#pragma once

#include "hal2/systick/systick.h"
#include "hal2/time_type.h"

namespace tvsc::hal2::systick {

class SysTickStm32l4xx final : public SysTickType {
 public:
  SysTickStm32l4xx() noexcept;
  [[nodiscard]] TimeType current_time_micros() const noexcept override;
  void increment_micros(TimeType us) noexcept override;
};

extern void handle_systick_interrupt() noexcept;

}  // namespace tvsc::hal2::systick
