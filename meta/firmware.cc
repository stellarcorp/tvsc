#include "meta/firmware.h"

#include <cstddef>
#include <cstdint>

#include "meta/flash.h"

namespace tvsc::meta {

extern "C" {
extern const char FIRMWARE_BEGIN;
extern const char FIRMWARE_END;
}

const uint32_t* const firmware_start{reinterpret_cast<const uint32_t*>(&FIRMWARE_BEGIN)};

const size_t firmware_size_pages{
    (static_cast<size_t>(&FIRMWARE_END - &FIRMWARE_BEGIN) + FLASH_PAGE_SIZE_BYTES - 1) /
    FLASH_PAGE_SIZE_BYTES};

const size_t firmware_size{firmware_size_pages * FLASH_PAGE_SIZE_BYTES};

}  // namespace tvsc::meta
