#pragma once

#include <cstdint>

#include "hal2/time_type.h"

namespace tvsc::hal2::systick {

class SysTickType {
 public:
  virtual ~SysTickType() = default;
  virtual TimeType current_time_micros() = 0;
  virtual void increment_micros(TimeType us) = 0;
};

}  // namespace tvsc::hal2::systick
