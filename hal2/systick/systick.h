#pragma once

#include <cstdint>

#include "hal2/time_type.h"

namespace tvsc::hal2::systick {

class SysTickType {
 public:
  virtual ~SysTickType() = default;
  [[nodiscard]] virtual TimeType current_time_micros() const noexcept = 0;
  virtual void increment_micros(TimeType us) noexcept = 0;

  /**
   * Mask the SysTick interrupt, turning off the ability to track time with this peripheral.
   */
  virtual void mask_interrupt() noexcept = 0;
  /**
   * Unmask the SysTick interrupt, re-enabling it after a previous call mask_interrupt().
   */
  virtual void unmask_interrupt() noexcept = 0;
};

}  // namespace tvsc::hal2::systick
