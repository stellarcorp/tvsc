#include "serial_wire/serial_wire.h"

#include <chrono>
#include <cstdint>

#include "time/embedded_clock.h"

extern "C" {
__attribute__((section(".status.value"))) uint32_t serial_wire_ack{};
__attribute__((section(".status.value"))) uint32_t attempt_count{};
}

namespace tvsc::serial_wire {

using namespace std::chrono_literals;

[[nodiscard]] Ack SerialWire::send_command(uint8_t command, bool retry) {
  static constexpr uint32_t MAX_ATTEMPTS{8};

  // uint8_t attempt_count{0};
  for (attempt_count = 0; attempt_count < MAX_ATTEMPTS; ++attempt_count) {
    programmer_.send_no_parity(command, 8);
    programmer_.receive_no_parity(serial_wire_ack, 3);
    if (retry && serial_wire_ack == ACK_RETRY) {
      programmer_.idle(2 * attempt_count + 8);
    } else {
      break;
    }
  }

  return static_cast<Ack>(serial_wire_ack);
}

[[nodiscard]] Ack SerialWire::read(uint8_t command, uint32_t& data, bool retry) {
  Ack result = send_command(command, retry);

  if (result == Ack::OK) {
    if (!programmer_.receive(data, 32)) {
      result = Ack::PARITY_FAIL;
    }
  }

  return result;
}

[[nodiscard]] Ack SerialWire::write(uint8_t command, uint32_t data, bool retry) {
  Ack result = send_command(command, retry);

  if (result == Ack::OK) {
    programmer_.send(data, 32);
  }

  return result;
}

/**
 * Reset the target board.
 */
void SerialWire::reset_target() {
  programmer_.initiate_target_board_reset();
  time::EmbeddedClock::clock().wait(programmer_.RESET_HOLD_TIME);
  programmer_.conclude_target_board_reset();
}

/**
 * Initialize, or re-initialize, the target to accept SWD commands.
 */
uint32_t SerialWire::initialize_swd() {
  // Special value to switch from JTAG to SWD. This value is part of the SWD specification. It can
  // also be found on page 1577 of the datasheet
  // https://www.st.com/resource/en/reference_manual/rm0394-stm32l41xxx42xxx43xxx44xxx45xxx46xxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf.
  // Note that the datasheet gives the bits in an order such that the MSB needs to be sent first.
  // Our send routines send the LSB first. This value is reversed from the one in the datasheet.
  static constexpr uint16_t JTAG_SWD_SWITCHING_SEQUENCE{0xe79e};

  // The SWD spec requires at least 50 clock cycles with SWDIO high to trigger the JTAG/SWD switch
  // sequence. We send multiple 32-bit values of all ones to accomplish this reset.
  programmer_.send_no_parity(std::numeric_limits<uint32_t>::max(), 32);
  programmer_.send_no_parity(std::numeric_limits<uint32_t>::max(), 32);

  // Send the magic value to switch from JTAG to SWD.
  programmer_.send_no_parity(JTAG_SWD_SWITCHING_SEQUENCE, 8 * sizeof(JTAG_SWD_SWITCHING_SEQUENCE));

  // Again, clock the SWCLK line for at least 50 clock cycles with SWDIO high followed by 4 cycles
  // with SWDIO driven low.
  programmer_.send_no_parity(std::numeric_limits<uint32_t>::max(), 32);
  programmer_.send_no_parity(std::numeric_limits<uint32_t>::max(), 32);
  programmer_.send_no_parity(0x00U, 4);

  // The serial wire specification requires reading the id before the target device is fully
  // switched from JTAG to SWD.

  uint32_t swd_target_idcode{0};
  if (swd_dp_read(0x00, swd_target_idcode) != Ack::OK) {
    swd_target_idcode = 0;
  }

  return swd_target_idcode;
}

}  // namespace tvsc::serial_wire
