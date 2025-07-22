#include "meta/firmware.h"

#include <cstddef>
#include <string_view>

namespace tvsc::meta {

extern "C" {
extern const char FIRMWARE_BEGIN;
extern const char FIRMWARE_END;
extern const char FLASH_PAGE_SIZE_BYTES;
}
const size_t flash_page_size_bytes{reinterpret_cast<size_t>(&FLASH_PAGE_SIZE_BYTES)};

const uint32_t* const firmware_start{reinterpret_cast<const uint32_t*>(&FIRMWARE_BEGIN)};

const size_t firmware_num_pages{
    (static_cast<size_t>(&FIRMWARE_END - &FIRMWARE_BEGIN) + flash_page_size_bytes - 1) /
    flash_page_size_bytes};

const size_t firmware_size{firmware_num_pages * flash_page_size_bytes};

}  // namespace tvsc::meta
