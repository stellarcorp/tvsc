#pragma once

#include "hal2/systick/systick.h"
#include "hal2/time_type.h"

namespace tvsc::hal2::systick {

class SysTickStm32l4xx final : public SysTickType {
 public:
  TimeType current_time_micros() override;
  void increment_micros(TimeType us) override;
};

extern void handle_systick_interrupt();

}  // namespace tvsc::hal2::systick
