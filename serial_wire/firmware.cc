#include "serial_wire/firmware.h"

#include <cstdint>

#include "serial_wire/serial_wire.h"

namespace tvsc::serial_wire {

bool flash_firmware(SerialWire& swd, size_t size, const uint8_t* firmware) {
  uint32_t data;

  // 1. Select AHB-AP (APSEL=0, Bank=0)
  if (!swd.swd_dp_write(0x08, 0x00000000)) {
    return false;
  }

  // 2. Halt the core
  if (!swd.swd_ap_write(0x04, 0xe000edf0)) {
    // TAR -> DHCSR
    return false;
  }
  if (!swd.swd_ap_write(0x0c, 0xa05f0003)) {
    // DRW -> HALT + DEBUGEN
    return false;
  }

  // 3. Unlock flash
  if (!swd.swd_ap_write(0x04, 0x40022008)) {
    // TAR -> FLASH_KEYR
    return false;
  }
  if (!swd.swd_ap_write(0x0c, 0x45670123)) {
    return false;
  }
  if (!swd.swd_ap_write(0x0c, 0xcdef89ab)) {
    return false;
  }

  // 4. Erase the flash using a mass erase.
  // TODO(james): Investigate if we can speed up the process using page-based erase.
  if (!swd.swd_ap_write(0x04, 0x40022014)) {
    // FLASH_CR
    return false;
  }
  if (!swd.swd_ap_write(0x0c, 1 << 2)) {
    // MER1
    return false;
  }
  if (!swd.swd_ap_write(0x0c, (1 << 2) | (1 << 16))) {
    // STRT
    return false;
  }

  // Wait for BSY to clear
  do {
    if (!swd.swd_ap_write(0x04, 0x40022010)) {
      // FLASH_SR
      return false;
    }
    if (!swd.swd_ap_read(0x0c, data)) {
      return false;
    }
  } while (data & (1 << 16));

  // 5. Program flash, aligning on 64-bit boundary.
  for (uint32_t addr = 0; addr < size; addr += 8) {
    uint64_t word = *(const uint64_t*)(firmware + addr);

    // Enable programming
    if (!swd.swd_ap_write(0x04, 0x40022014)) {
      // FLASH_CR
      return false;
    }
    if (!swd.swd_ap_write(0x0c, 1)) {
      // PG
      return false;
    }

    // Write target address
    // TODO(james): Determine if there is some form of autoincrementing facility for this address.
    if (!swd.swd_ap_write(0x04, 0x08000000 + addr)) {
      return false;
    }

    // Write data.
    if (!swd.swd_ap_write(0x0c, (uint32_t)(word & 0xffffffff))) {
      return false;
    }
    if (!swd.swd_ap_write(0x0c, (uint32_t)(word >> 32))) {
      return false;
    }

    // Wait for BSY to clear
    do {
      if (!swd.swd_ap_write(0x04, 0x40022010)) {
        return false;
      }
      if (!swd.swd_ap_read(0x0c, data)) {
        return false;
      }
    } while (data & (1 << 16));
  }

  // 6. Disable programming
  if (!swd.swd_ap_write(0x04, 0x40022014)) {
    return false;
  }
  if (!swd.swd_ap_write(0x0c, 0x00000000)) {
    return false;
  }

  // 7. Lock flash
  if (!swd.swd_ap_write(0x0c, 1 << 31)) {
    return false;
  }

  // 8. Resume execution
  if (!swd.swd_ap_write(0x04, 0xe000edf0)) {
    return false;
  }
  if (!swd.swd_ap_write(0x0c, 0xa05f0001)) {
    return false;
  }

  return true;
}

}  // namespace tvsc::serial_wire
