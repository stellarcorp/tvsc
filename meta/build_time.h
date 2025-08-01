#pragma once

#include <cstdint>

#include "hal/time_type.h"

namespace tvsc::meta {

struct BuildTime final {
  hal::TimeType timestamp;
};

extern const BuildTime BUILD_TIME;
extern const uint32_t BUILD_TIME_ADDR;

}  // namespace tvsc::meta
