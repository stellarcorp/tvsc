#pragma once

#include <chrono>

#include "bits/bits.h"
#include "hal/programmer/programmer.h"

namespace tvsc::serial_wire {

using namespace std::chrono_literals;

enum class Ack {
  OK = 1,
  WAIT = 2,
  ERROR = 4,
  FAIL = 7,
  PARITY_FAIL = 8,
  ID_MISMATCH = 9,
  INITIALIZATION_ERROR = 10,
  TOO_MANY_ATTEMPTS = 11,
};

class Result final {
 public:
  Ack ack{Ack::OK};

  Result() = default;
  Result(Ack a) : ack(a) {}
  Result(const Result& rhs) : ack(rhs.ack) {}
  Result(Result&& rhs) : ack(rhs.ack) {}

  Result& operator=(const Result& rhs) {
    ack = rhs.ack;
    return *this;
  }
  Result& operator=(Result&& rhs) {
    ack = rhs.ack;
    return *this;
  }

  Result& operator=(Ack a) {
    ack = a;
    return *this;
  }

  explicit operator bool() const { return ack == Ack::OK; }

  bool operator==(const Result& rhs) const { return ack == rhs.ack; }
  bool operator==(Ack rhs) const { return ack == rhs; }

  bool operator!=(const Result& rhs) const { return ack != rhs.ack; }
  bool operator!=(Ack rhs) const { return ack != rhs; }
};

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

  [[nodiscard]] Ack send_command(uint8_t command, bool retry);

  [[nodiscard]] Ack read(uint8_t command, uint32_t& data, bool retry);
  [[nodiscard]] Ack write(uint8_t command, uint32_t data, bool retry);

 public:
  SerialWire(tvsc::hal::programmer::ProgrammerPeripheral& programmer_peripheral,
             std::chrono::nanoseconds clock_period = 1us)
      : programmer_(programmer_peripheral.access()) {
    programmer_.set_clock_period(clock_period);
  }

  void reset_target();

  /**
   * Initialize, or re-initialize, the target to accept SWD commands.
   */
  [[nodiscard]] uint32_t initialize_swd();

  [[nodiscard]] Result swd_ap_read(uint8_t addr, uint32_t& data, bool retry = true) {
    return read(create_command(BLANK_ACCESS_PORT_READ_COMMAND, addr), data, retry);
  }

  [[nodiscard]] Result swd_ap_write(uint8_t addr, uint32_t data, bool retry = true) {
    return write(create_command(BLANK_ACCESS_PORT_WRITE_COMMAND, addr), data, retry);
  }

  [[nodiscard]] Result swd_dp_read(uint8_t addr, uint32_t& data, bool retry = true) {
    return read(create_command(BLANK_DEBUG_PORT_READ_COMMAND, addr), data, retry);
  }

  [[nodiscard]] Result swd_dp_write(uint8_t addr, uint32_t data, bool retry = true) {
    return write(create_command(BLANK_DEBUG_PORT_WRITE_COMMAND, addr), data, retry);
  }
};

}  // namespace tvsc::serial_wire
