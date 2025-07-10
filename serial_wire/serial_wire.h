#pragma once

#include <chrono>

#include "bits/bits.h"
#include "hal/programmer/programmer.h"

namespace tvsc::serial_wire {

using namespace std::chrono_literals;

class SerialWire final {
 private:
  tvsc::hal::programmer::Programmer programmer_;

  static constexpr uint8_t BLANK_ACCESS_PORT_READ_COMMAND{0b10000111};
  static constexpr uint8_t BLANK_ACCESS_PORT_WRITE_COMMAND{0b10000011};
  static constexpr uint8_t BLANK_DEBUG_PORT_READ_COMMAND{0b10000101};
  static constexpr uint8_t BLANK_DEBUG_PORT_WRITE_COMMAND{0b10000001};
  static constexpr uint8_t ACK_OK{0x01};
  static constexpr uint8_t ACK_RETRY{0x02};

  static inline void compute_command_parity(uint8_t& command) {
    uint8_t parity{};
    for (uint8_t i = 1; i < 5; ++i) {
      parity += bits::get_bit_field_value<1>(command, i);
    }
    bits::modify_bit_field<1, 5>(command, parity);
  }

  static inline void add_address(uint8_t& command, uint8_t addr) {
    addr = addr >> 2;
    bits::modify_bit_field<2, 3>(command, addr);
  }

  static inline uint8_t create_command(uint8_t command_template, uint8_t addr) {
    uint8_t command{command_template};
    add_address(command, addr);
    compute_command_parity(command);
    return command;
  }

  [[nodiscard]] bool send_command(uint8_t command);

  [[nodiscard]] bool read(uint8_t command, uint32_t& data);
  [[nodiscard]] bool write(uint8_t command, uint32_t data);

 public:
  // Debug port registers.

  // Clear sticky error flags.
  static constexpr uint8_t DP_ABORT{0x00};
  // Control/status flags and power requests.
  static constexpr uint8_t DP_CTRL_STAT{0x04};
  // Select AP and AP register bank.
  static constexpr uint8_t DP_SELECT{0x08};
  // Read buffer (for AP read result).
  static constexpr uint8_t DP_RDBUFF{0x0C};
  // [On DPv1] ID register (overlaps with ABORT).
  static constexpr uint8_t DP_IDCODE{0x00};
  // [On DPv2+] Replaces IDCODE.
  static constexpr uint8_t DP_TARGETID{0x00};

  static constexpr uint32_t EXPECTED_SW_DP_IDCODE{0x2BA01477};

  SerialWire(tvsc::hal::programmer::ProgrammerPeripheral& programmer_peripheral,
             std::chrono::nanoseconds clock_period = 100us)
      : programmer_(programmer_peripheral.access()) {
    programmer_.set_clock_period(clock_period);
  }

  void reset_target();

  /**
   * Initialize, or re-initialize, the target to accept SWD commands.
   */
  [[nodiscard]] uint32_t initialize_swd();

  bool clear_dp_state() { return swd_dp_write(DP_ABORT, 0x1E) && swd_dp_write(DP_SELECT, 0x00); }

  [[nodiscard]] bool swd_ap_read(uint8_t addr, uint32_t& data) {
    return read(create_command(BLANK_ACCESS_PORT_READ_COMMAND, addr), data);
  }

  [[nodiscard]] bool swd_ap_write(uint8_t addr, uint32_t data) {
    return write(create_command(BLANK_ACCESS_PORT_WRITE_COMMAND, addr), data);
  }

  [[nodiscard]] bool swd_dp_read(uint8_t addr, uint32_t& data) {
    return read(create_command(BLANK_DEBUG_PORT_READ_COMMAND, addr), data);
  }

  [[nodiscard]] bool swd_dp_write(uint8_t addr, uint32_t data) {
    return write(create_command(BLANK_DEBUG_PORT_WRITE_COMMAND, addr), data);
  }
};

}  // namespace tvsc::serial_wire
