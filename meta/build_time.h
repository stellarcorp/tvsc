#pragma once

#include <cstdint>

namespace tvsc::meta {

struct BuildTime final {
  uint32_t timestamp;
};

extern const BuildTime BUILD_TIME;
extern const uint32_t BUILD_TIME_ADDR;

}  // namespace tvsc::meta
