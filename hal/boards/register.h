#pragma once

#include <cstdint>

#include "base/bits.h"

namespace tvsc::hal::boards {

class Register final {
 private:
  // Note that a register's value is not initialized when the class is instantiated. If the value
  // was initialized, it would overwrite the value in the hardware register, including possibly
  // writing to private or undefined bits. Boards and register banks will need special logic to
  // bring all of the registers to valid initial states.
  volatile uint32_t value_;

  /**
   * Perform a dummy read on the register to block until the value of the register has been updated.
   */
  void block_until_updated() const volatile {
    volatile uint32_t temp = value_;
    (void)temp;
  }

 public:
  /**
   * Read the value of the register.
   */
  const volatile uint32_t& value() const volatile { return value_; }

  /**
   * Set the value of the register.
   */
  void set_value(uint32_t value) volatile { value_ = value; }

  /**
   * Set the value of the register and block until the value is read back, guaranteeing that the
   * write is complete.
   */
  void set_value_and_block(uint32_t value) volatile {
    value_ = value;
    block_until_updated();
  }

  /**
   * Read the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  uint32_t bit_field_value() const volatile {
    return get_bit_field_value<NUM_BITS, BIT_OFFSET>(value_);
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_bit_field_value(uint32_t bit_field_value) volatile {
    modify_bit_field<NUM_BITS, BIT_OFFSET>(value_, bit_field_value);
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_bit_field_value_and_block(uint32_t bit_field_value) volatile {
    modify_bit_field<NUM_BITS, BIT_OFFSET>(value_, bit_field_value);
    block_until_updated();
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS>
  void set_bit_field_value(uint32_t bit_field_value, uint8_t bit_field_offset) volatile {
    modify_bit_field<NUM_BITS>(value_, bit_field_value, bit_field_offset);
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS>
  void set_bit_field_value_and_block(uint32_t bit_field_value, uint8_t bit_field_offset) volatile {
    modify_bit_field<NUM_BITS>(value_, bit_field_value, bit_field_offset);
    block_until_updated();
  }

  /**
   * Set the value of a bit field in a register.
   */
  void set_bit_field_value(uint32_t bit_field_value, uint8_t num_bits,
                           uint8_t bit_field_offset) volatile {
    modify_bit_field(value_, bit_field_value, num_bits, bit_field_offset);
  }

  /**
   * Set the value of a bit field in a register.
   */
  void set_bit_field_value_and_block(uint32_t bit_field_value, uint8_t num_bits,
                                     uint8_t bit_field_offset) volatile {
    modify_bit_field(value_, bit_field_value, num_bits, bit_field_offset);
    block_until_updated();
  }
};

}  // namespace tvsc::hal::boards
