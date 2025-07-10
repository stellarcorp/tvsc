#include "meta/firmware.h"

#include <cstddef>
#include <string_view>

namespace tvsc::meta {

extern "C" {
extern const char* const FIRMWARE_BEGIN;
extern const char* const FIRMWARE_END;
}

const std::string_view firmware{FIRMWARE_BEGIN, static_cast<size_t>(FIRMWARE_END - FIRMWARE_BEGIN)};

}  // namespace tvsc::meta
