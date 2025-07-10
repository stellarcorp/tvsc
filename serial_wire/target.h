#pragma once

#include <array>
#include <cstring>
#include <span>

#include "serial_wire/serial_wire.h"

extern "C" {
__attribute__((section(".status.value"))) int fail_step{0};
__attribute__((section(".status.value"))) uint32_t initialized_idcode{};
__attribute__((section(".status.value"))) uint32_t target_idcode{};
__attribute__((section(".status.value"))) uint32_t target_ap_id{};
__attribute__((section(".status.value"))) uint32_t target_dp_status{};
}

namespace tvsc::serial_wire {

class Target final {
 private:
  // Access port registers.
  // Control/Status Word.
  static constexpr uint8_t CSW{0x00};

  // Transfer Address Register
  static constexpr uint8_t TAR{0x04};

  // Data Read/Write register
  static constexpr uint8_t DRW{0x0c};

  // ID Register (IDR)
  static constexpr uint8_t IDR{0xfc};

  SerialWire* swd_;
  bool swd_initialized_{false};

  [[nodiscard]] bool initialize_target() {
    fail_step = 0;
    initialized_idcode = swd_->initialize_swd();
    if (initialized_idcode == 0) {
      fail_step = 1;
      return false;
    }

    if (!swd_->swd_dp_read(SerialWire::DP_IDCODE, target_idcode)) {
      fail_step = 10;
      return false;
    }

    if ((target_idcode != SerialWire::EXPECTED_SW_DP_IDCODE)) {
      fail_step = 11;
      return false;
    }

    // Clear any errors on the DP.
    if (!swd_->swd_dp_write(SerialWire::DP_ABORT, 0x1e)) {
      fail_step = 12;
      return false;
    }

    // Select bank 0.
    if (!swd_->swd_dp_write(SerialWire::DP_SELECT, 0x00)) {
      fail_step = 13;
      return false;
    }

    // Issue system power request and debug power up request.
    if (!swd_->swd_dp_write(SerialWire::DP_CTRL_STAT, 0x50000000)) {
      fail_step = 110;
      return false;
    }

    // Wait for start-up acknowledgements.
    static constexpr uint32_t MAX_RETRIES{32};
    uint32_t attempt_count{};
    do {
      if (!swd_->swd_dp_read(SerialWire::DP_CTRL_STAT, target_dp_status)) {
        fail_step = 113;
        return false;
      }

      if ((target_dp_status & 0x30)) {
        if (!swd_->swd_dp_write(SerialWire::DP_ABORT, 0x1e)) {
          fail_step = 114;
          return false;
        }
      }
      ++attempt_count;
    } while ((target_dp_status & 0xa0000030) != 0xa0000000 && attempt_count < MAX_RETRIES);

    if (attempt_count >= MAX_RETRIES) {
      fail_step = 115;
      return false;
    }

    // Read AP register 0xfc.
    // Select AP bank 0xf0.
    if (!swd_->swd_dp_write(SerialWire::DP_SELECT, 0xf0)) {
      fail_step = 117;
      return false;
    }
    // Read AP 0xfc.
    if (!swd_->swd_ap_read(0xfc, target_ap_id)) {
      fail_step = 118;
      return false;
    }

    // Set CSW to 32-bit access, auto-increment on, AHB-AP
    if (!swd_->swd_dp_write(SerialWire::DP_SELECT, 0x00)) {
      fail_step = 120;
      return false;
    }
    if (!swd_->swd_ap_write(CSW, 0x02)) {
      fail_step = 125;
      return false;
    }

    return true;
  }

  [[nodiscard]] bool ensure_initialized() {
    if (!swd_initialized_) {
      if (!initialize_target()) {
        return false;
      }
      swd_initialized_ = true;
    }
    return true;
  }

 public:
  Target(SerialWire& swd) : swd_(&swd) {}

  [[nodiscard]] bool read_dp_idcode(uint32_t& id) {
    if (!ensure_initialized()) {
      return false;
    }

    if (!swd_->swd_dp_read(SerialWire::DP_IDCODE, id)) {
      return false;
    }

    return true;
  }

  [[nodiscard]] bool read_ap_idr(uint32_t& id) {
    if (!ensure_initialized()) {
      return false;
    }

    bool success{true};

    // Read AP register 0xfc.
    // Select AP bank 0xf0.
    if (success && !swd_->swd_dp_write(SerialWire::DP_SELECT, 0xf0)) {
      success = false;
    }

    // Read AP IDR.
    if (success && !swd_->swd_ap_read(IDR, id)) {
      success = false;
    }

    return swd_->clear_dp_state() && success;
  }
};

}  // namespace tvsc::serial_wire
