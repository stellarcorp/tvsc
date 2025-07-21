#include "meta/firmware.h"

#include <cstddef>
#include <string_view>

namespace tvsc::meta {

extern "C" {
extern const char FIRMWARE_BEGIN;
extern const char FIRMWARE_END;
}

const uint32_t* const firmware_start{reinterpret_cast<const uint32_t*>(&FIRMWARE_BEGIN)};
const size_t firmware_size{static_cast<size_t>(&FIRMWARE_END - &FIRMWARE_BEGIN)};

}  // namespace tvsc::meta
