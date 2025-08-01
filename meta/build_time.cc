#include "meta/build_time.h"

#include <cstdint>

namespace tvsc::meta {

__attribute__((section(".build_time"))) const BuildTime BUILD_TIME{
    BUILD_TIMESTAMP,  // Symbol defined by Bazel during build.
};

__attribute__((section(".build_time")))
const uint32_t BUILD_TIME_ADDR{reinterpret_cast<uint32_t>(&BUILD_TIME)};

}  // namespace tvsc::meta
