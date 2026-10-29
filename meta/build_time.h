#pragma once

#include <cstdint>

#include "hal2/time_type.h"

namespace tvsc::meta {

struct BuildTime final {
  hal2::TimeType timestamp;
};

extern const BuildTime BUILD_TIME;
extern const uint32_t BUILD_TIME_ADDR;

}  // namespace tvsc::meta
