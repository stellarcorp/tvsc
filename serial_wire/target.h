#pragma once

#include <cstring>

#include "bits/bits.h"
#include "serial_wire/serial_wire.h"

extern "C" {
extern __attribute__((section(".status.value"))) int fail_step;
extern __attribute__((section(".status.value"))) uint32_t target_dp_dpidr;
extern __attribute__((section(".status.value"))) uint32_t target_dp_abort;
extern __attribute__((section(".status.value"))) uint32_t target_dp_ctrl_stat;
extern __attribute__((section(".status.value"))) uint32_t target_dp_resend;
extern __attribute__((section(".status.value"))) uint32_t target_dp_select;
extern __attribute__((section(".status.value"))) uint32_t target_dp_rdbuff;
extern __attribute__((section(".status.value"))) uint32_t target_ap_status;
extern __attribute__((section(".status.value"))) uint32_t dummy;
extern __attribute__((section(".status.value"))) uint32_t mem_addr;
}

namespace tvsc::serial_wire {

class Target final {
 public:
 private:
  SerialWire* swd_;
  bool swd_initialized_{false};
  bool ap_initialized_{false};
  uint32_t select_value_{};

  [[nodiscard]] Result initialize_target();

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

  [[nodiscard]] Result clear_errors();

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

  [[nodiscard]] Result select_ap(uint8_t ap);
  [[nodiscard]] Result select_ap(uint8_t ap, uint8_t bank);
  [[nodiscard]] Result select_ap_bank(uint8_t bank);

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

  [[nodiscard]] Result read_ap_id(uint32_t& value);

  [[nodiscard]] Result ap_read_mem(uint32_t addr, uint32_t& value);
  [[nodiscard]] Result ap_write_mem(uint32_t addr, uint32_t value);

  [[nodiscard]] Result ap_read_mem(uint32_t addr, uint32_t* destination, size_t length);
  [[nodiscard]] Result ap_write_mem(uint32_t addr, const uint32_t* source, size_t length);

  [[nodiscard]] Result power_on();
  [[nodiscard]] Result power_off();

  [[nodiscard]] Result enable_debug() {
    // Enable debug.
    // The top four nibbles of the value are the "debug key" OF 0Xa05f. This value is required to be
    // written by the debugger / programmer to get write access to the lower four nibbles. The lower
    // four nibbles turn on the DEBUGEN flag; the zero bits have no effect.
    return ap_write_mem(/*DHCSR address*/ 0xE000EDF0, 0xa05f0001);
  }

  [[nodiscard]] Result disable_debug() {
    return ap_write_mem(/*DHCSR address*/ 0xE000EDF0, 0xa05f0000);
  }
};

}  // namespace tvsc::serial_wire
