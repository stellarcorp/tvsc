#include "hal/programmer/stm32l4xx_programmer.h"

#include "bits/bits.h"
#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::programmer {

using namespace tvsc::hal::gpio;

void ProgrammerStm32l4xx::enable() {
  gpio_ = gpio_peripheral_->access();
  gpio_.set_pin_mode(swclk_pin_, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);
}

void ProgrammerStm32l4xx::disable() {
  idle(1);
  turnaround(SwdioDriveState::FLOAT);

  gpio_.set_pin_mode(reset_pin_, PinMode::UNUSED);
  gpio_.set_pin_mode(swclk_pin_, PinMode::UNUSED);
  gpio_.set_pin_mode(swdio_pin_, PinMode::UNUSED);

  gpio_.invalidate();
}

void ProgrammerStm32l4xx::initiate_target_board_reset() {
  gpio_.set_pin_mode(reset_pin_, PinMode::OUTPUT_OPEN_DRAIN, PinSpeed::LOW);
  gpio_.write_pin(reset_pin_, 0);
}

void ProgrammerStm32l4xx::conclude_target_board_reset() {
  gpio_.write_pin(reset_pin_, 1);
  half_period_delay();
  half_period_delay();
  gpio_.set_pin_mode(reset_pin_, PinMode::UNUSED);
}

void ProgrammerStm32l4xx::set_clock_period(std::chrono::nanoseconds clock_period) {
  half_clock_period_cycles_ = SystemCoreClock * clock_period.count() / (2 * 1e9);
}

void ProgrammerStm32l4xx::turnaround(SwdioDriveState state) {
  if (current_swdio_drive_state_ == state) {
    return;
  }

  current_swdio_drive_state_ = state;
  if (state == SwdioDriveState::FLOAT) {
    gpio_.set_pin_mode(swdio_pin_, swdio_input_mode_, PinSpeed::LOW);
  }
  half_period_delay();

  gpio_.write_pin(swclk_pin_, 1);

  half_period_delay();

  gpio_.write_pin(swclk_pin_, 0);

  if (state == SwdioDriveState::DRIVE) {
    gpio_.set_pin_mode(swdio_pin_, swdio_output_mode_, PinSpeed::LOW);
  }
}

// Send the bits_to_send LSBs of data. Do not send any parity bits.
void ProgrammerStm32l4xx::send_no_parity(uint32_t data, uint8_t bits_to_send) {
  turnaround(SwdioDriveState::DRIVE);
  for (uint8_t i = 0; i < bits_to_send; ++i) {
    gpio_.write_pin(swdio_pin_, bits::get_bit_field_value<1>(data, i));

    half_period_delay();

    gpio_.write_pin(swclk_pin_, 1);

    half_period_delay();

    gpio_.write_pin(swclk_pin_, 0);
  }
}

// Send the bits_to_send LSBs of data followed by a parity bit.
void ProgrammerStm32l4xx::send(uint32_t data, uint8_t bits_to_send) {
  const uint8_t parity{static_cast<uint8_t>(__builtin_parity(data & ((1U << bits_to_send) - 1)))};
  send_no_parity(data, bits_to_send);

  // Send the parity.
  gpio_.write_pin(swdio_pin_, parity & 0x01);

  half_period_delay();

  gpio_.write_pin(swclk_pin_, 1);

  half_period_delay();

  gpio_.write_pin(swclk_pin_, 0);
}

// Receive the specified number of bits into data. The target will not send any parity bits during
// this transfer.
void ProgrammerStm32l4xx::receive_no_parity(uint32_t& data, uint8_t bits_to_receive) {
  turnaround(SwdioDriveState::FLOAT);

  data = 0;

  for (uint8_t i = 0; i < bits_to_receive; ++i) {
    half_period_delay();

    const uint8_t bit_value{gpio_.read_pin(swdio_pin_)};
    bits::modify_bit_field<1>(data, static_cast<uint32_t>(bit_value), i);

    gpio_.write_pin(swclk_pin_, 1);

    half_period_delay();

    gpio_.write_pin(swclk_pin_, 0);
  }
}

// Receive the specified number of bits into data along with a single bit of parity. Returns true
// if the parity matches the data received.
bool ProgrammerStm32l4xx::receive(uint32_t& data, uint8_t bits_to_receive) {
  receive_no_parity(data, bits_to_receive);

  half_period_delay();

  const uint8_t bit_value{gpio_.read_pin(swdio_pin_)};

  gpio_.write_pin(swclk_pin_, 1);

  half_period_delay();

  gpio_.write_pin(swclk_pin_, 0);

  const uint8_t parity{static_cast<uint8_t>(__builtin_parity(data))};
  return (parity & 0x01) == bit_value;
}

void ProgrammerStm32l4xx::idle(uint32_t num_cycles) {
  while (num_cycles > 0) {
    if (num_cycles < 32) {
      send_no_parity(0x0, num_cycles);
      num_cycles = 0;
    } else {
      send_no_parity(0x0, 32);
      num_cycles -= 32;
    }
  }
}

}  // namespace tvsc::hal::programmer
