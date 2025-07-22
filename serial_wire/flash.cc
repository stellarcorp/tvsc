#include "serial_wire/flash.h"

#include <cstdint>

#include "bits/bits.h"
#include "meta/flash.h"
#include "serial_wire/serial_wire.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

extern "C" {
__attribute__((section(".status.value"))) uint32_t current_page{};
__attribute__((section(".status.value"))) uint32_t num_pages{};
__attribute__((section(".status.value"))) uint32_t flash_operation_status{};
}

namespace tvsc::serial_wire {

[[nodiscard]] Result Flash::wait_while_busy() {
  Result success{};
  uint32_t flash_status{};
  do {
    success = target_->ap_read_mem(FLASH_SR, flash_status);
  } while (success && (flash_status & FLASH_SR_BSY));
  return success;
}

[[nodiscard]] Result Flash::erase_page_unlocked(uint8_t page) {
  Result success{};

  uint32_t flash_ctrl{};
  if (success) {
    success = wait_while_busy();
  }

  // Set page erase and page number
  if (success) {
    flash_ctrl = 0;
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }

  if (success) {
    bits::modify_bit_field<8, 3>(flash_ctrl, static_cast<uint32_t>(page));  // flash page
    bits::modify_bit_field<1, 1>(flash_ctrl, 1UL);                          // page erase
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  // Start erase
  if (success) {
    bits::modify_bit_field<1, 16>(flash_ctrl, 1UL);  // start flag
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  if (success) {
    success = wait_while_busy();
  }

  // Clear erase flags
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }
  if (success) {
    bits::modify_bit_field<1, 2>(flash_ctrl, 0UL);  // mass erase
    bits::modify_bit_field<1, 1>(flash_ctrl, 0UL);  // page erase
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  return success;
}

[[nodiscard]] Result Flash::mass_erase_unlocked() {
  Result success{};

  uint32_t flash_ctrl{};
  if (success) {
    success = wait_while_busy();
  }

  // Set mass erase flag
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }

  if (success) {
    bits::modify_bit_field<1, 2>(flash_ctrl, 1UL);  // mass erase
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  // Start erase
  if (success) {
    bits::modify_bit_field<1, 16>(flash_ctrl, 1UL);  // start flag
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  if (success) {
    success = wait_while_busy();
  }

  // Clear erase flags
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }
  if (success) {
    bits::modify_bit_field<1, 2>(flash_ctrl, 0UL);  // mass erase
    bits::modify_bit_field<1, 1>(flash_ctrl, 0UL);  // page erase
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }

  return success;
}

[[nodiscard]] Result Flash::unlock_flash() {
  Result success{};
  // Unlock Flash. The flash is locked after every reset. It cannot be erased or programmed until
  // it is unlocked by two successive writes of special values to the KEY register.
  if (success) {
    success = target_->ap_write_mem(FLASH_KEYR, FLASH_KEY1);
  }
  if (success) {
    success = target_->ap_write_mem(FLASH_KEYR, FLASH_KEY2);
  }
  return success;
}

[[nodiscard]] Result Flash::lock_flash() {
  Result success{};
  // Lock the flash. Not strictly necessary, as it gets locked after every reset, but it avoids
  // potential (though unlikely) issues where the flash could be overwritten.
  uint32_t flash_ctrl{};
  if (success) {
    success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
  }
  if (success) {
    bits::modify_bit_field<1, 31>(flash_ctrl, 1UL);
    success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
  }
  return success;
}

[[nodiscard]] Result Flash::mass_erase() {
  Result success{};
  if (success) {
    success = unlock_flash();
  }
  if (success) {
    success = mass_erase_unlocked();
  }
  if (success) {
    success = lock_flash();
  }
  return success;
}

[[nodiscard]] Result Flash::erase_page(uint8_t page) {
  Result success{};
  if (success) {
    success = unlock_flash();
  }
  if (success) {
    success = erase_page_unlocked(page);
  }
  if (success) {
    success = lock_flash();
  }
  return success;
}

[[nodiscard]] Result Flash::write_entire_flash(const uint32_t* source, uint32_t length) {
  Result success{};

  num_pages = (length * sizeof(uint32_t) + tvsc::meta::FLASH_PAGE_SIZE_BYTES - 1) /
              tvsc::meta::FLASH_PAGE_SIZE_BYTES;

  if (success) {
    success = unlock_flash();
  }

  if (success) {
    success = mass_erase_unlocked();
  }

  for (current_page = 0; success && current_page < num_pages; ++current_page) {
    if (success) {
      success = wait_while_busy();
    }

    // Set the page number and set the fast programming bit (FSTPG). Clear the page erase bit.
    uint32_t flash_ctrl{};
    if (success) {
      success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
    }
    if (success) {
      bits::modify_bit_field<8, 3>(flash_ctrl, current_page);  // flash page
      bits::modify_bit_field<1, 1>(flash_ctrl, 0UL);           // page erase
      bits::modify_bit_field<1, 18>(flash_ctrl, 1UL);          // fast programming
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    // Write a page 64 words at a time.
    for (size_t i = 0; success && i < tvsc::meta::FLASH_PAGE_SIZE_BYTES / 64; ++i) {
      success = target_->ap_write_mem(
          FLASH_BASE + current_page * tvsc::meta::FLASH_PAGE_SIZE_BYTES + i * 64 * sizeof(uint32_t),
          source + current_page * (tvsc::meta::FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)) + i * 64,
          64);

      // Read the status whether we have success or not to check for errors or the end of operation
      // flag.
      do {
        // Repeat read of status register until we no longer get a busy status (BSY flag).
        (void)target_->ap_read_mem(FLASH_SR, flash_operation_status);
      } while (bits::get_bit_field_value<1, 16>(flash_operation_status));

      // Check for errors and clear them.
      if (bits::get_bit_field_value<1, 1>(flash_operation_status)) {
        // Operation error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 3>(flash_operation_status)) {
        // Programming error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 4>(flash_operation_status)) {
        // Write-protection error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 5>(flash_operation_status)) {
        // Alignment error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 6>(flash_operation_status)) {
        // Size error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 7>(flash_operation_status)) {
        // Programming sequence error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 8>(flash_operation_status)) {
        // Timing miss error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 9>(flash_operation_status)) {
        // Fast programming error.
        success = Ack::ERROR;
      }

      // Successful write including end of operation flag.
      if (success && bits::get_bit_field_value<1, 0>(flash_operation_status)) {
        // The EOP and the various error flags are cleared by writing a 1 to them.
        bits::modify_bit_field<1, 0>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 1>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 3>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 4>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 5>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 6>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 7>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 8>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 9>(flash_operation_status, 1UL);
        success = target_->ap_write_mem(FLASH_SR, flash_operation_status);
      }

      // Clear the programming bits.
      if (success) {
        success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
      }
      if (success) {
        bits::modify_bit_field<8, 3>(flash_ctrl, current_page);  // flash page
        bits::modify_bit_field<1, 18>(flash_ctrl, 0UL);          // fast programming
        bits::modify_bit_field<1, 0>(flash_ctrl, 0UL);           // page programming
        success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
      }
    }
  }

  if (success) {
    success = wait_while_busy();
  }

  if (success) {
    success = lock_flash();
  }

  return success;
}

[[nodiscard]] Result Flash::write_pages(uint8_t start_page, uint8_t num_pages,
                                        const uint32_t* source) {
  Result success{};

  if (success) {
    success = unlock_flash();
  }

  for (current_page = start_page; success && current_page < start_page + num_pages;
       ++current_page) {
    if (success) {
      success = wait_while_busy();
    }

    if (success) {
      success = erase_page_unlocked(current_page);
    }

    // Set the page number and set the fast programming bit (FSTPG). Clear the page erase bit.
    uint32_t flash_ctrl{};
    if (success) {
      success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
    }
    if (success) {
      bits::modify_bit_field<8, 3>(flash_ctrl, current_page);  // flash page
      bits::modify_bit_field<1, 1>(flash_ctrl, 0UL);           // page erase
      bits::modify_bit_field<1, 0>(flash_ctrl, 1UL);           // normal programming
      // bits::modify_bit_field<1, 18>(flash_ctrl, 1UL);       // fast programming
      success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
    }

    // Write a page in batches.
    static constexpr size_t BATCH_SIZE_WORDS{tvsc::meta::FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)};
    for (size_t i = 0;
         success && i < tvsc::meta::FLASH_PAGE_SIZE_BYTES / (BATCH_SIZE_WORDS * sizeof(uint32_t));
         ++i) {
      success = target_->ap_write_mem(
          FLASH_BASE + current_page * tvsc::meta::FLASH_PAGE_SIZE_BYTES +
              i * BATCH_SIZE_WORDS * sizeof(uint32_t),
          source + current_page * (tvsc::meta::FLASH_PAGE_SIZE_BYTES / sizeof(uint32_t)) +
              i * BATCH_SIZE_WORDS,
          BATCH_SIZE_WORDS);

      // Read the status whether we have success or not to check for errors or the end of operation
      // flag.
      do {
        // Repeat read of status register until we no longer get a busy status (BSY flag).
        (void)target_->ap_read_mem(FLASH_SR, flash_operation_status);
      } while (bits::get_bit_field_value<1, 16>(flash_operation_status));

      // Check for errors and clear them.
      if (bits::get_bit_field_value<1, 1>(flash_operation_status)) {
        // Operation error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 3>(flash_operation_status)) {
        // Programming error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 4>(flash_operation_status)) {
        // Write-protection error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 5>(flash_operation_status)) {
        // Alignment error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 6>(flash_operation_status)) {
        // Size error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 7>(flash_operation_status)) {
        // Programming sequence error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 8>(flash_operation_status)) {
        // Timing miss error.
        success = Ack::ERROR;
      } else if (bits::get_bit_field_value<1, 9>(flash_operation_status)) {
        // Fast programming error.
        success = Ack::ERROR;
      }

      // Successful write including end of operation flag.
      if (success && bits::get_bit_field_value<1, 0>(flash_operation_status)) {
        // The EOP and the various error flags are cleared by writing a 1 to them.
        bits::modify_bit_field<1, 0>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 1>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 3>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 4>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 5>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 6>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 7>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 8>(flash_operation_status, 1UL);
        bits::modify_bit_field<1, 9>(flash_operation_status, 1UL);
        success = target_->ap_write_mem(FLASH_SR, flash_operation_status);
      }

      // Clear the programming bits.
      if (success) {
        success = target_->ap_read_mem(FLASH_CR, flash_ctrl);
      }
      if (success) {
        bits::modify_bit_field<8, 3>(flash_ctrl, current_page);  // flash page
        bits::modify_bit_field<1, 18>(flash_ctrl, 0UL);          // fast programming
        bits::modify_bit_field<1, 0>(flash_ctrl, 0UL);           // page programming
        success = target_->ap_write_mem(FLASH_CR, flash_ctrl);
      }
    }
  }

  if (success) {
    success = wait_while_busy();
  }

  if (success) {
    success = lock_flash();
  }

  return success;
}

}  // namespace tvsc::serial_wire
