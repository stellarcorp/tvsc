#include "serial_wire/flash.h"

#include <cstdint>

#include "bits/bits.h"
#include "serial_wire/serial_wire.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

extern "C" {
__attribute__((section(".status.value"))) uint32_t current_page{};
__attribute__((section(".status.value"))) uint32_t num_pages{};
}

namespace tvsc::serial_wire {

/**
 * Erase a range of flash memory pages on the target device over SWD.
 *
 * This function takes a starting address and length in bytes, then calculates the page-aligned
 * range of flash memory that needs to be erased. It communicates with the target's FLASH
 * peripheral using memory-mapped SWD AP accesses to unlock the flash control interface,
 * initiate page erasure, and monitor the busy and error flags in the FLASH_SR register.
 *
 * The procedure includes writing two key values to the FLASH_KEYR register to unlock
 * the flash, setting the PER bit in the FLASH_CR register to enable page erase mode, and starting
 * the erase by setting the STRT bit. After each page erase, it waits for the BSY bit to clear,
 * checks for errors, and moves on to the next page. Once complete, it disables erase mode
 * and re-locks the flash control registers to protect against accidental modification.
 */
[[nodiscard]] Result Flash::erase_flash(uint32_t address, uint32_t length) {
  Result success{};

  // Round to page boundaries
  if (address < FLASH_BASE_ADDRESS) {
    success = Ack::INITIALIZATION_ERROR;
    return success;
  }
  uint32_t page_start = (address - FLASH_BASE_ADDRESS) / FLASH_PAGE_SIZE;
  uint32_t num_pages = (length + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

  // Unlock Flash.
  if (success) {
    success = target_->ap_write_mem(FLASH_KEYR, FLASH_KEY1);
  }
  if (success) {
    success = target_->ap_write_mem(FLASH_KEYR, FLASH_KEY2);
  }

  uint32_t flash_status{};
  uint32_t flash_ctrl{};
  for (current_page = page_start; success && current_page < page_start + num_pages;
       ++current_page) {
    // Wait for BSY = 0
    do {
      success = target_->ap_read_mem(FLASH_SR, flash_status);
    } while (success && (flash_status & FLASH_SR_BSY));

    // Set PER and page number
    if (success) {
      flash_ctrl = 0;
      success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
    }

    if (success) {
      bits::modify_bit_field<1, 1>(flash_ctrl, 1UL);
      bits::modify_bit_field<8, 3>(flash_ctrl, current_page);
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    // Start erase
    if (success) {
      bits::modify_bit_field<1, 16>(flash_ctrl, 1UL);
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    // Wait for BSY = 0
    if (success) {
      do {
        success = target_->ap_read_mem(FLASH_SR, flash_status);
      } while (success && (flash_status & FLASH_SR_BSY));
    }

    // Clear PER bit
    if (success) {
      success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
    }
    if (success) {
      bits::modify_bit_field<1, 1>(flash_ctrl, 0UL);
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }
  }

  // Lock Flash
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }
  if (success) {
    bits::modify_bit_field<1, 31>(flash_ctrl, 1UL);
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  return success;
}

[[nodiscard]] Result Flash::write_flash(uint32_t address, const uint32_t* source, uint32_t length) {
  Result success{};

  // Round to page boundaries and do some fundamental error checks, especially around alignment.
  if (address < FLASH_BASE_ADDRESS) {
    success = Ack::INITIALIZATION_ERROR;
    return success;
  }

  num_pages = (length + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

  // Unlock Flash. The flash is locked after every reset. It cannot be erased or programmed until
  // it is unlocked by two successive writes of special values to the KEY register.
  if (success) {
    success = target_->ap_write_mem(FLASH_KEYR, FLASH_KEY1);
  }
  if (success) {
    success = target_->ap_write_mem(FLASH_KEYR, FLASH_KEY2);
  }

  uint32_t flash_status{};
  uint32_t flash_ctrl{};

  // Mass erase.
  // Wait until the flash is not busy (BSY bit is zero).
  // if (success) {
  //   do {
  //     success = target_->ap_read_mem(FLASH_SR, flash_status);
  //   } while (success && (flash_status & FLASH_SR_BSY));
  // }

  // Set mass erase flag (MER1)
  // if (success) {
  //   success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  // }
  // if (success) {
  //   bits::modify_bit_field<1, 2>(flash_ctrl, 1UL);
  //   success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  // }

  // // Start the erase.
  // if (success) {
  //   bits::modify_bit_field<1, 16>(flash_ctrl, 1UL);
  //   success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  // }

  // // Wait until the flash is not busy (BSY bit is zero).
  // if (success) {
  //   do {
  //     success = target_->ap_read_mem(FLASH_SR, flash_status);
  //   } while (success && (flash_status & FLASH_SR_BSY));
  // }

  // // Clear the mass erase bit (MER1).
  // if (success) {
  //   success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  // }
  // if (success) {
  //   bits::modify_bit_field<1, 2>(flash_ctrl, 0UL);
  //   success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  // }

  // // Wait until the flash is not busy (BSY bit is zero).
  // if (success) {
  //   do {
  //     success = target_->ap_read_mem(FLASH_SR, flash_status);
  //   } while (success && (flash_status & FLASH_SR_BSY));
  // }

  for (current_page = 0; success && current_page < num_pages; ++current_page) {
    // Set the page number and the page erase bit. Clear the programming bit.
    if (success) {
      success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
    }
    if (success) {
      bits::modify_bit_field<8, 3>(flash_ctrl, current_page);
      bits::modify_bit_field<1, 1>(flash_ctrl, 1UL);
      bits::modify_bit_field<1, 0>(flash_ctrl, 0UL);
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    // Start the erase.
    if (success) {
      bits::modify_bit_field<1, 16>(flash_ctrl, 1UL);
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    // Wait until the flash is not busy (BSY bit is zero).
    if (success) {
      do {
        success = target_->ap_read_mem(FLASH_SR, flash_status);
      } while (success && (flash_status & FLASH_SR_BSY));
    }

    // Set the page number and set the programming bit (PG). Clear the page erase bit.
    if (success) {
      success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
    }
    if (success) {
      bits::modify_bit_field<8, 3>(flash_ctrl, current_page);
      bits::modify_bit_field<1, 1>(flash_ctrl, 0UL);
      bits::modify_bit_field<1, 0>(flash_ctrl, 1UL);
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    if (success) {
      success = target_->ap_write_mem(address + current_page * FLASH_PAGE_SIZE,
                                      source + (current_page * FLASH_PAGE_SIZE) / sizeof(uint32_t),
                                      FLASH_PAGE_SIZE);
    }
    if (success) {
      do {
        success = target_->ap_read_mem(FLASH_SR, flash_status);
      } while (success && (flash_status & FLASH_SR_BSY));
    }
  }

  // Write the page of data 32 double words at a time to the flash address.
  // if (success) {
  //   for (uint32_t i = 0; success && i < FLASH_PAGE_SIZE / 64; ++i) {
  //     success = target_->ap_write_mem(
  //         address + current_page * FLASH_PAGE_SIZE + i * 64,
  //         source + (current_page * FLASH_PAGE_SIZE + i * 64) / sizeof(uint32_t), 64);
  // Wait until the flash is not busy (BSY bit is zero).
  // Also wait until EOP is set but maybe only if some special interrupts are enabled?
  // if (success) {
  //   do {
  //     success = target_->ap_read_mem(FLASH_SR, flash_status);
  //   } while (success && (flash_status & FLASH_SR_BSY) && !(flash_status & FLASH_SR_EOP));
  // }

  // // Clear the EOP flag.
  // bits::modify_bit_field<1, 0>(flash_status, 0UL);
  // success = target_->ap_write_mem(FLASH_SR, flash_status);
  // if (success) {
  //   do {
  //     success = target_->ap_read_mem(FLASH_SR, flash_status);
  //   } while (success && (flash_status & FLASH_SR_BSY));
  // }
  //     }
  //   }
  // }

  // Wait until the flash is not busy (BSY bit is zero).
  if (success) {
    do {
      success = target_->ap_read_mem(FLASH_SR, flash_status);
    } while (success && (flash_status & FLASH_SR_BSY));
  }

  // Clear the programming bit.
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }
  if (success) {
    bits::modify_bit_field<1, 18>(flash_ctrl, 0UL);
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  // Lock the flash. Not strictly necessary, as it gets locked after every reset, but it avoids
  // potential (though unlikely) issues where the flash could be overwritten.
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }
  if (success) {
    bits::modify_bit_field<1, 31>(flash_ctrl, 1UL);
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  return success;
}

}  // namespace tvsc::serial_wire
