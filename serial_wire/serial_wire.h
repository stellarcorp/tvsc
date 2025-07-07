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

  static inline void compute_command_parity(uint8_t& command) {
    uint8_t parity{};
    for (uint8_t i = 1; i < 5; ++i) {
      parity += bits::get_bit_field_value<1>(command, i);
    }
    bits::modify_bit_field<1, 5>(command, parity);
  }

  static inline void add_address(uint8_t& command, uint8_t addr) {
    command |= ((addr & 0x03) << 3);
  }

  static inline uint8_t create_command(uint8_t command_template, uint8_t addr) {
    uint8_t command{command_template};
    add_address(command, addr);
    compute_command_parity(command);
    return command;
  }

  [[nodiscard]] bool read(uint8_t command, uint32_t& data);
  [[nodiscard]] bool write(uint8_t command, uint32_t data);

 public:
  SerialWire(tvsc::hal::programmer::ProgrammerPeripheral& programmer_peripheral,
             std::chrono::nanoseconds clock_period = 100us)
      : programmer_(programmer_peripheral.access()) {
    programmer_.set_clock_period(clock_period);
  }

  uint32_t read_id_code() {
    uint32_t id{};
    swd_dp_read(0x00, id);
    return id;
  }

  void reset_target();

  /**
   * Initialize, or re-initialize, the target to accept SWD commands.
   */
  uint32_t initialize_swd();

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
