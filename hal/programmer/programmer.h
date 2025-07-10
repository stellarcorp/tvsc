#pragma once

#include <chrono>
#include <cstdint>

#include "hal/peripheral.h"

namespace tvsc::hal::programmer {

using namespace std::chrono_literals;

class Programmer;

enum class SwdioDriveState {
  FLOAT = 0,
  DRIVE = 1,
};

class ProgrammerPeripheral : public Peripheral<ProgrammerPeripheral, Programmer> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void initiate_target_board_reset() = 0;
  virtual void conclude_target_board_reset() = 0;

  friend class Programmer;

  // Set the period of the serial wire clock signal.
  virtual void set_clock_period(std::chrono::nanoseconds clock_period) = 0;

  // Send the bits_to_send LSBs of data. Do not send any parity bits.
  virtual void send_no_parity(uint32_t data, uint8_t bits_to_send) = 0;

  // Send the bits_to_send LSBs of data followed by a parity bit.
  virtual void send(uint32_t data, uint8_t bits_to_send) = 0;

  // Put the SWDIO line in the specified state and cycle the clock signal.
  virtual void turnaround(SwdioDriveState state) = 0;

  // Receive the specified number of bits into data. The target will not send any parity bits during
  // this transfer.
  virtual void receive_no_parity(uint32_t& data, uint8_t bits_to_receive) = 0;

  // Receive the specified number of bits into data along with a single bit of parity. Returns true
  // if the parity matches the data received.
  virtual bool receive(uint32_t& data, uint8_t bits_to_receive) = 0;

  // Cycle the clock the given number of cycles while leaving the output line to float.
  virtual void idle(uint32_t num_cycles) = 0;

 public:
  virtual ~ProgrammerPeripheral() = default;
};

class Programmer final : public Functional<ProgrammerPeripheral, Programmer> {
 private:
  Programmer(ProgrammerPeripheral& peripheral)
      : Functional<ProgrammerPeripheral, Programmer>(peripheral) {}

  friend class Peripheral<ProgrammerPeripheral, Programmer>;

 public:
  Programmer() = default;

  static constexpr std::chrono::milliseconds RESET_HOLD_TIME{50ms};
  void initiate_target_board_reset() { return peripheral_->initiate_target_board_reset(); }
  void conclude_target_board_reset() { return peripheral_->conclude_target_board_reset(); }

  void set_clock_period(std::chrono::nanoseconds clock_period) {
    return peripheral_->set_clock_period(clock_period);
  }

  void send_no_parity(uint32_t data, uint8_t bits_to_send) {
    return peripheral_->send_no_parity(data, bits_to_send);
  }

  void send(uint32_t data, uint8_t bits_to_send) { return peripheral_->send(data, bits_to_send); }

  void idle(SwdioDriveState state) { return peripheral_->turnaround(state); }

  void receive_no_parity(uint32_t& data, uint8_t bits_to_receive) {
    return peripheral_->receive_no_parity(data, bits_to_receive);
  }

  bool receive(uint32_t& data, uint8_t bits_to_receive) {
    return peripheral_->receive(data, bits_to_receive);
  }

  void idle(uint32_t num_cycles) { return peripheral_->idle(num_cycles); }
};

}  // namespace tvsc::hal::programmer
