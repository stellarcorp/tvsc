#pragma once

#include <cstdint>

#include "hal/time_type.h"

namespace tvsc::hal::systick {

class SysTickType {
 public:
  virtual ~SysTickType() = default;
  virtual TimeType current_time_micros() = 0;
  virtual void increment_micros(TimeType us) = 0;
  virtual void handle_interrupt() = 0;
};

}  // namespace tvsc::hal::systick
