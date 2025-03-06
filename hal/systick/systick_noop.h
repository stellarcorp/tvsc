#pragma once

#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

class SysTickNoop final : public SysTickType {
 public:
  TimeType current_time_micros() override;
  void increment_micros(TimeType us) override;
  void handle_interrupt() override;
};

}  // namespace tvsc::hal::systick
