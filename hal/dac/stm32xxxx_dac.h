#pragma once

#include <cstdint>
#include <new>

#include "hal/dac/dac.h"
#include "hal/register.h"

namespace tvsc::hal::dac {

class DacRegisterBank final {
 public:
  // Offset 0x00
  volatile Register CR;

  // Offset 0x04
  volatile Register SWTRGR;

  // Right-aligned (small value) 12-bit value holding register for Channel 1.
  // Offset 0x08
  volatile Register DHR12R1;

  // Left-aligned (small value) 12-bit value holding register for Channel 1.
  // Offset 0x0c
  volatile Register DHR12L1;

  // 8-bit value holding register for Channel 1.
  // Offset 0x10
  volatile Register DHR8R1;

  // Right-aligned (small value) 12-bit value holding register for Channel 2.
  // Offset 0x14
  volatile Register DHR12R2;

  // Left-aligned (small value) 12-bit value holding register for Channel 2.
  // Offset 0x18
  volatile Register DHR12L2;

  // 8-bit value holding register for Channel 2.
  // Offset 0x1c
  volatile Register DHR8R2;
};

template <uint8_t CHANNEL_INDEX = 0>
class DacStm32xxxx;

template <>
class DacStm32xxxx<0 /* CHANNEL_INDEX */> final : public Dac {
 private:
  DacRegisterBank* registers_;

 public:
  DacStm32xxxx(void* base_address) : registers_(new (base_address) DacRegisterBank) {
    // Configure the DAC for software triggering.
    registers_->CR.set_bit_field_value<3, 3>(0b111);
    // Disable triggering by external signals.
    registers_->CR.set_bit_field_value<1, 2>(0b0);
  }

  void set_value(uint8_t value) override {
    // Turn on the DAC. This sets the enable bit on the CR register for this channel.
    registers_->CR.set_bit_field_value<1, 0>(0b1);

    registers_->DHR8R1.set_bit_field_value<8, 0>(value);

    // Trigger the DAC.
    registers_->SWTRGR.set_bit_field_value<1, 0>(1);
  }

  void clear_value() override {
    // Turn off the DAC.
    registers_->CR.set_bit_field_value<1, 0>(0b0);
  }
};

template <>
class DacStm32xxxx<1 /* CHANNEL_INDEX */> final : public Dac {
 private:
  DacRegisterBank* registers_;

 public:
  DacStm32xxxx(void* base_address) : registers_(new (base_address) DacRegisterBank) {
    // Configure the DAC for software triggering.
    registers_->CR.set_bit_field_value<3, 3 + 16>(0b111);
    // Disable triggering by external signals.
    registers_->CR.set_bit_field_value<1, 2 + 16>(0b0);
  }

  void set_value(uint8_t value) override {
    // Turn on the DAC. This sets the enable bit on the CR register for this channel.
    registers_->CR.set_bit_field_value<1, 16>(0b1);

    registers_->DHR8R2.set_bit_field_value<8, 0>(value);

    // Trigger the DAC.
    registers_->SWTRGR.set_bit_field_value<1, 1>(1);
  }

  void clear_value() override {
    // Turn off the DAC.
    registers_->CR.set_bit_field_value<1, 16>(0b0);
  }
};

}  // namespace tvsc::hal::dac
