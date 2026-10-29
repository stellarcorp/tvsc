#include "meta/build_time.h"

#include <cstdint>

#include "platforms/linker_sections.h"

namespace tvsc::meta {

KEEP_IN_SECTION(BUILD_TIME_SECTION)
const BuildTime BUILD_TIME{
    BUILD_TIMESTAMP,  // Symbol defined by Bazel during build.
};

KEEP_IN_SECTION(BUILD_TIME_ADDR_SECTION)
const uint32_t BUILD_TIME_ADDR{reinterpret_cast<uint32_t>(&BUILD_TIME)};

}  // namespace tvsc::meta
