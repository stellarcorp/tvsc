#include "serial_wire/target.h"

#include <cstring>

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

static constexpr uint32_t EXPECTED_SW_DP_IDCODE{0x2BA01477};

[[nodiscard]] Result Target::initialize_target() {
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

[[nodiscard]] Result Target::clear_errors() {
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
[[nodiscard]] Result Target::power_on() {
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
[[nodiscard]] Result Target::power_off() {
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

[[nodiscard]] Result Target::select_ap(uint8_t ap, uint8_t bank) {
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

[[nodiscard]] Result Target::select_ap(uint8_t ap) {
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

[[nodiscard]] Result Target::select_ap_bank(uint8_t bank) {
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

[[nodiscard]] Result Target::read_ap_id(uint32_t& value) {
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

[[nodiscard]] Result Target::ap_read_mem(uint32_t addr, uint32_t& value) {
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

[[nodiscard]] Result Target::ap_write_mem(uint32_t addr, uint32_t value) {
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
    success = ap_write_register(AP_DRW, value);
    if (!success) {
      fail_step = 5030;
    }
  }

  if (success) {
    // Issue an AP read that blocks until the previous transaction completes. This should flush
    // the previous write.
    success = ap_read_register(AP_CSW, dummy);
    if (!success) {
      fail_step = 9040;
    }
  }

  return success;
}

[[nodiscard]] Result Target::ap_read_mem(uint32_t addr, uint32_t* destination, size_t size) {
  Result success{};
  mem_addr = addr;
  if (success) {
    success = ap_write_register(AP_CSW, 0x23000052);
    if (!success) {
      fail_step = 8000;
    }
  }

  if (success) {
    success = ap_write_register(AP_TAR, addr);
    if (!success) {
      fail_step = 8010;
    }
  }

  if (success) {
    success = ap_read_register(AP_DRW, dummy);
    if (!success) {
      fail_step = 8030;
    }
  }

  for (size_t i = 0; i < size - 1; ++i) {
    if (success) {
      success = ap_read_register(AP_DRW, *(destination + i));
      if (!success) {
        fail_step = 8040;
      }
    }
  }

  if (success) {
    success = read_rdbuff(*(destination + size - 1));
    if (!success) {
      fail_step = 8070;
    }
  }

  return success;
}

[[nodiscard]] Result Target::ap_write_mem(uint32_t addr, const uint32_t* source, size_t size) {
  Result success{};
  mem_addr = addr;
  if (success) {
    success = ap_write_register(AP_CSW, 0x23000052);
    if (!success) {
      fail_step = 8000;
    }
  }

  if (success) {
    success = ap_write_register(AP_TAR, addr);
    if (!success) {
      fail_step = 8010;
    }
  }

  for (size_t i = 0; success && i < size; ++i) {
    if (success) {
      success = ap_write_register(AP_DRW, *(source + i));
      if (!success) {
        fail_step = 8040;
      }
    }
  }

  if (success) {
    // Issue an AP read that blocks until the previous transaction completes. This should flush
    // the previous writes.
    success = ap_read_register(AP_CSW, dummy);
    if (!success) {
      fail_step = 8050;
    }
  }

  return success;
}

}  // namespace tvsc::serial_wire
