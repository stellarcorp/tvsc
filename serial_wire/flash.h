#pragma once

#include <cstdint>

#include "bits/bits.h"
#include "serial_wire/serial_wire.h"
#include "serial_wire/target.h"

namespace tvsc::serial_wire {

class Flash final {
 private:
  Target* target_;

  static constexpr uint32_t FLASH_BASE_ADDRESS{0x08000000};

  // Register to unlock the flash control interface by accepting a specific key sequence.
  static constexpr uint32_t FLASH_KEYR{0x40022008};

  // Status register indicating ongoing operations and errors.
  static constexpr uint32_t FLASH_SR{0x40022010};

  // Configuration register to configure and trigger operations like erase.
  static constexpr uint32_t FLASH_CR{0x40022014};

  // Register to configure user option bytes including read protection, watchdog mode, and boot
  // settings.
  // static constexpr uint32_t FLASH_OPTR = 0x40022020;

  // Register write values to unlock flash control register access when issued in sequence.
  // static constexpr uint32_t FLASH_KEY1 = 0x45670123;
  // static constexpr uint32_t FLASH_KEY2 = 0xCDEF89AB;

  // Bit in FLASH_SR indicating a flash operation is in progress
  // static constexpr uint32_t FLASH_SR_BSY = 1 << 16;

  // Bit in FLASH_CR to enable page erase mode
  // static constexpr uint32_t FLASH_CR_PER = 1 << 1;

  // Bit in FLASH_CR to start an erase operation
  // static constexpr uint32_t FLASH_CR_STRT = 1 << 16;

  // Bit in FLASH_CR to enable programming mode (not used in erase)
  // static constexpr uint32_t FLASH_CR_PG = 1 << 0;

  // Bit in FLASH_CR to lock the flash control register after operations
  // static constexpr uint32_t FLASH_CR_LOCK = 1 << 31;

  // Position of the page number field within FLASH_CR (bits 10:3)
  // static constexpr uint32_t FLASH_CR_PAGE_SHIFT{3};

  [[nodiscard]] Result wait_while_busy();

  [[nodiscard]] Result mass_erase_unlocked();
  [[nodiscard]] Result erase_page_unlocked(uint8_t page);

  [[nodiscard]] Result lock_flash();
  [[nodiscard]] Result unlock_flash();

 public:
  Flash(Target& target) : target_(&target) {}

  [[nodiscard]] Result mass_erase();
  [[nodiscard]] Result erase_page(uint8_t page);

  [[nodiscard]] Result write_entire_flash(const uint32_t* source, uint32_t length);
  [[nodiscard]] Result write_pages(uint8_t page, uint8_t num_pages, const uint32_t* source);
};

}  // namespace tvsc::serial_wire
