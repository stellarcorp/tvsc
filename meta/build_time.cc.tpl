// Auto-generated file — do not edit manually

#include "meta/build_time.h"

#include <cstdint>

namespace tvsc::meta {

__attribute__((section(".build_time"))) const BuildTime BUILD_TIME {
  @BUILD_TIME_EPOCH@U,
  "@BUILD_TIME_STR@",
};

__attribute__((section(".build_time"))) const uint32_t BUILD_TIME_ADDR{reinterpret_cast<uint32_t>(&BUILD_TIME)};

}  // namespace tvsc::meta
