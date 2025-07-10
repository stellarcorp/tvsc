#pragma once

#include <cstdint>

namespace tvsc::meta {

struct BuildTime final {
  uint32_t timestamp;
  // The build time string below uses ISO 8601 with precision to the second. Example:
  // "2025-07-07T14:58:03Z". This string will require 20 bytes plus one byte of null-termination.
  char build_time_str[21];
};

extern const BuildTime BUILD_TIME;
extern const uint32_t BUILD_TIME_ADDR;

}  // namespace tvsc::meta
