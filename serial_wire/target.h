#pragma once

#include <array>
#include <cstring>
#include <span>

#include "bits/bits.h"
#include "serial_wire/serial_wire.h"

extern "C" {
__attribute__((section(".status.value"))) int fail_step{0};
__attribute__((section(".status.value"))) uint32_t target_dp_dpidr{};
__attribute__((section(".status.value"))) uint32_t target_dp_abort{};
__attribute__((section(".status.value"))) uint32_t target_dp_ctrl_stat{};
__attribute__((section(".status.value"))) uint32_t target_dp_resend{};
__attribute__((section(".status.value"))) uint32_t target_dp_select{};
__attribute__((section(".status.value"))) uint32_t target_dp_rdbuff{};
__attribute__((section(".status.value"))) uint32_t target_ap_status{};
__attribute__((section(".status.value"))) uint32_t dummy{};
__attribute__((section(".status.value"))) uint32_t mem_addr{};
}

namespace tvsc::serial_wire {

class Target final {
 public:
  static constexpr uint32_t EXPECTED_SW_DP_IDCODE{0x2BA01477};

 private:
  SerialWire* swd_;
  bool swd_initialized_{false};
  bool ap_initialized_{false};
  uint32_t select_value_{};

  [[nodiscard]] Result initialize_target() {
    Result result{};
    fail_step = 0;
    uint32_t initialized_idcode = swd_->initialize_swd();
    if (initialized_idcode == 0) {
      fail_step = 1;
      result = Ack::INITIALIZATION_ERROR;
    }

    if (result) {
      result = read_idr(target_dp_dpidr);
      if (!result) {
        fail_step = 10;
      }
    }

    if (result) {
      if ((target_dp_dpidr != EXPECTED_SW_DP_IDCODE)) {
        fail_step = 15;
        result = Ack::ID_MISMATCH;
      }
    }

    if (result) {
      result = power_on();
    }

    return result;
  }

  // Debug port registers.
  // IHI0031G_debug_interface_v5_2_architecture_specification.pdf retrieved via
  // https://developer.arm.com/documentation/ihi0031/latest/
  // See page B2-50 for the address map corresponding to DPv1.
  static constexpr uint8_t DP_DPIDR{0x00};
  static constexpr uint8_t DP_ABORT{0x00};
  static constexpr uint8_t DP_CTRL_STAT{0x04};
  static constexpr uint8_t DP_RESEND{0x08};
  static constexpr uint8_t DP_SELECT{0x08};
  static constexpr uint8_t DP_RDBUFF{0x0C};

  // Access port registers.
  // Control/Status Word.
  static constexpr uint8_t AP_CSW{0x00};

  // Transfer Address Register
  static constexpr uint8_t AP_TAR{0x04};

  // Data Read/Write register
  static constexpr uint8_t AP_DRW{0x0c};

  // ID Register (IDR)
  static constexpr uint8_t AP_IDR{0xfc};

 public:
  Target(SerialWire& swd) : swd_(&swd) { (void)ensure_initialized(); }

  [[nodiscard]] Result ensure_initialized() {
    Result result{};
    if (!swd_initialized_) {
      result = initialize_target();
      if (result) {
        swd_initialized_ = true;
      }
    }
    return result;
  }

  bool is_initialized() const { return swd_initialized_; }

  // Low-level register accessors. This API implements DPv0 and DPv1, excepting the DLCR (Data Link
  // Control Register) which we do not need at the moment.
  // IHI0031G_debug_interface_v5_2_architecture_specification.pdf retrieved via
  // https://developer.arm.com/documentation/ihi0031/latest/
  // See page B2-47 for reference details of the Debug Port in general.
  // See page B2-53 for register descriptions.

  [[nodiscard]] Result write_abort(uint32_t value) {
    Result success{swd_->swd_dp_write(DP_ABORT, value)};
    if (success) {
      target_dp_abort = value;
    }
    return success;
  }

  [[nodiscard]] Result read_ctrl_stat(uint32_t& value) {
    Result success{swd_->swd_dp_read(DP_CTRL_STAT, value)};
    if (success) {
      target_dp_ctrl_stat = value;
    }
    return success;
  }

  [[nodiscard]] Result write_ctrl_stat(uint32_t value) {
    Result success{swd_->swd_dp_write(DP_CTRL_STAT, value)};
    if (success) {
      target_dp_ctrl_stat = value;
    }
    return success;
  }

  [[nodiscard]] Result read_idr(uint32_t& value) {
    Result success{swd_->swd_dp_read(DP_DPIDR, value)};
    if (success) {
      target_dp_dpidr = value;
    }
    return success;
  }

  [[nodiscard]] Result read_resend(uint32_t& value) {
    Result success{swd_->swd_dp_read(DP_RESEND, value)};
    if (success) {
      target_dp_resend = value;
    }
    return success;
  }

  [[nodiscard]] Result write_select(uint32_t value) {
    Result success{};
    if (value != select_value_) {
      success = swd_->swd_dp_write(DP_SELECT, value);
      if (success) {
        select_value_ = value;
        target_dp_select = value;
      }
    }
    return success;
  }

  [[nodiscard]] Result read_rdbuff(uint32_t& value) {
    Result success{swd_->swd_dp_read(DP_RDBUFF, value)};
    if (success) {
      target_dp_rdbuff = value;
    }
    return success;
  }

  // Higher level accessors implementing common functionality.
  [[nodiscard]] bool has_sticky_errors() {
    uint32_t ctrl_status_value{};
    Result success{read_ctrl_stat(ctrl_status_value)};
    if (!success) {
      fail_step = 150;
      return true;
    }
    return (ctrl_status_value & 0xb0) != 0;
  }

  [[nodiscard]] Result clear_errors() {
    static constexpr uint32_t MAX_RETRIES{8};
    uint32_t ctrl_status_value{};
    uint32_t attempt_count{};
    Result result;
    do {
      result = read_ctrl_stat(ctrl_status_value);
      if (!result) {
        fail_step = 105;
        return result;
      }

      if ((ctrl_status_value & 0xb0)) {
        result = write_abort(0x1e);
        if (!result) {
          fail_step = 110;
          return result;
        }
      }
      ++attempt_count;
    } while ((ctrl_status_value & 0xb0) != 0x00 && attempt_count < MAX_RETRIES);

    if (attempt_count >= MAX_RETRIES) {
      // Every read and write succeeds, but we still have errors in the CTRL/STAT register.
      result = Ack::TOO_MANY_ATTEMPTS;
      fail_step = 120;
    }

    return result;
  }

  // Turn on both system and debug power domains.
  [[nodiscard]] Result power_on() {
    uint32_t ctrl_status_value{};
    Result result{};
    result = read_ctrl_stat(ctrl_status_value);
    if (!result) {
      fail_step = 205;
      return result;
    }
    bits::modify_bit_field<1, 28, uint32_t>(ctrl_status_value, 1U);
    bits::modify_bit_field<1, 30, uint32_t>(ctrl_status_value, 1U);

    // Issue system power request and debug power up request.
    result = write_ctrl_stat(ctrl_status_value);
    if (!result) {
      fail_step = 210;
      return result;
    }

    // Wait for start-up acknowledgements.
    static constexpr uint32_t MAX_RETRIES{32};
    uint32_t attempt_count{};
    do {
      result = read_ctrl_stat(ctrl_status_value);
      if (!result) {
        fail_step = 215;
        return result;
      }

      if ((ctrl_status_value & 0x30)) {
        result = write_abort(0x1e);
        if (!result) {
          fail_step = 220;
          return result;
        }
      }
      ++attempt_count;
    } while ((ctrl_status_value & 0xa0000030) != 0xa0000000 && attempt_count < MAX_RETRIES);

    if (attempt_count >= MAX_RETRIES) {
      fail_step = 225;
      result = Ack::TOO_MANY_ATTEMPTS;
    }

    return result;
  }

  // Turn off both system and debug power domains.
  [[nodiscard]] Result power_off() {
    Result result{};
    result = clear_errors();
    if (!result) {
      return result;
    }

    uint32_t ctrl_status_value{};
    result = read_ctrl_stat(ctrl_status_value);
    if (!result) {
      fail_step = 305;
      return result;
    }
    bits::modify_bit_field<1, 28, uint32_t>(ctrl_status_value, 0);
    bits::modify_bit_field<1, 30, uint32_t>(ctrl_status_value, 0);

    // Issue system power request and debug power up request.
    result = write_ctrl_stat(ctrl_status_value);
    if (!result) {
      fail_step = 310;
      return result;
    }

    // Wait for shutdown acknowledgements.
    static constexpr uint32_t MAX_RETRIES{256};
    uint32_t attempt_count{};
    do {
      result = read_ctrl_stat(ctrl_status_value);
      if (!result) {
        fail_step = 315;
        return result;
      }

      if ((ctrl_status_value & 0x30)) {
        result = write_abort(0x1e);
        if (!result) {
          fail_step = 320;
          return result;
        }
      }
      ++attempt_count;
    } while ((ctrl_status_value & 0x80000030) != 0x00 && attempt_count < MAX_RETRIES);

    if (attempt_count >= MAX_RETRIES) {
      fail_step = 325;
      result = Ack::TOO_MANY_ATTEMPTS;
    }

    return result;
  }

  // Access port accessors.

  // Low-level register accessors.
  [[nodiscard]] Result ap_read_register(uint8_t addr, uint32_t& value, bool retry = true) {
    Result success{};
    success = select_ap_bank(addr & 0xf0);
    if (success) {
      success = swd_->swd_ap_read(addr & 0x0f, value, retry);
    }
    return success;
  }

  [[nodiscard]] Result ap_write_register(uint8_t addr, uint32_t value) {
    Result success{};
    success = select_ap_bank(addr & 0xf0);
    if (success) {
      success = swd_->swd_ap_write(addr & 0x0f, value);
    }
    return success;
  }

  [[nodiscard]] Result select_ap(uint8_t ap, uint8_t bank) {
    Result result{};
    const uint32_t value{(static_cast<uint32_t>(ap) << 24) | (bank & 0xf0)};
    if (value != select_value_) {
      result = write_select(value);
      if (result) {
        select_value_ = value;
      } else {
        fail_step = 600;
      }
    }
    return result;
  }

  [[nodiscard]] Result select_ap(uint8_t ap) {
    Result result{};
    uint32_t value{select_value_};
    bits::modify_bit_field<8, 24, uint32_t>(value, ap);
    if (value != select_value_) {
      result = write_select(value);
      if (result) {
        select_value_ = value;
      } else {
        fail_step = 610;
      }
    }
    return result;
  }

  [[nodiscard]] Result select_ap_bank(uint8_t bank) {
    bank = bank >> 4;
    Result result{};
    uint32_t value{select_value_};
    bits::modify_bit_field<4, 4, uint32_t>(value, bank);
    if (value != select_value_) {
      result = write_select(value);
      if (result) {
        select_value_ = value;
      } else {
        fail_step = 610;
      }
    }
    return result;
  }

  uint8_t ap() const {
    return static_cast<uint8_t>(bits::get_bit_field_value<8, 24>(select_value_));
  }

  uint8_t ap_bank() const {
    return static_cast<uint8_t>(bits::get_bit_field_value<8, 0>(select_value_));
  }

  // Higher level functional accessors.
  [[nodiscard]] Result initialize_ap_connection(uint8_t ap) {
    Result success{};
    success = select_ap(ap, 0);
    return success;
  }

  [[nodiscard]] Result read_ap_id(uint32_t& value) {
    Result success{};
    if (success) {
      success = ap_write_register(AP_CSW, 0x23000042);
      if (!success) {
        fail_step = 6000;
      }
    }
    if (success) {
      success = ap_read_register(AP_IDR, dummy);
      if (!success) {
        fail_step = 6010;
      }
    }

    if (success) {
      success = read_rdbuff(value);
      if (!success) {
        fail_step = 6020;
      }
    }
    return success;
  }

  [[nodiscard]] Result ap_read_mem(uint32_t addr, uint32_t& value) {
    Result success{};
    mem_addr = addr;
    if (success) {
      success = ap_write_register(AP_CSW, 0x23000042);
      if (!success) {
        fail_step = 5000;
      }
    }

    if (success) {
      success = ap_write_register(AP_TAR, addr);
      if (!success) {
        fail_step = 5010;
      }
    }

    if (success) {
      success = ap_read_register(AP_DRW, dummy);
      if (!success) {
        fail_step = 5030;
      }
    }

    if (success) {
      success = read_rdbuff(value);
      if (!success) {
        fail_step = 5070;
      }
    }

    return success;
  }

  [[nodiscard]] Result ap_read_mem(uint32_t addr, std::span<uint32_t> destination) {
    Result success{};
    mem_addr = addr;
    if (success) {
      success = ap_write_register(AP_CSW, 0x23000052);
      if (!success) {
        fail_step = 7000;
      }
    }

    if (success) {
      success = ap_write_register(AP_TAR, addr);
      if (!success) {
        fail_step = 7010;
      }
    }

    if (success) {
      success = ap_read_register(AP_DRW, dummy);
      if (!success) {
        fail_step = 7030;
      }
    }

    for (size_t i = 0; i < destination.size() - 1; ++i) {
      if (success) {
        success = ap_read_register(AP_DRW, destination[i]);
        if (!success) {
          fail_step = 7040;
        }
      }
    }

    if (success) {
      success = read_rdbuff(destination[destination.size() - 1]);
      if (!success) {
        fail_step = 7070;
      }
    }

    return success;
  }
};

}  // namespace tvsc::serial_wire
