#pragma once

#include <cstdint>

#include "serial_wire/serial_wire.h"

namespace tvsc::serial_wire {

/**
 * Flash the given bytes as firmware to the target board.
 */
bool flash_firmware(SerialWire& swd, size_t size, const uint8_t* firmware_bytes);

}  // namespace tvsc::serial_wire
