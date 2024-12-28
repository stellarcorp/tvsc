#pragma once

#include <cstdint>

namespace tvsc::hal::boards {

template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
constexpr uint32_t compute_mask() {
  return ((1U << NUM_BITS) - 1U) << BIT_OFFSET;
}

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
  void block_until_updated() const {
    uint32_t temp = value_;
    (void)temp;
  }

 public:
  /**
   * Read the value of the register.
   */
  uint32_t value() const { return value_; }

  /**
   * Set the value of the register.
   */
  void set_value(uint32_t value) { value_ = value; }

  /**
   * Set the value of the register and block until the value is read back, guaranteeing that the
   * write is complete.
   */
  void set_value_and_block(uint32_t value) {
    value_ = value;
    block_until_updated();
  }

  /**
   * Read the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  uint32_t field_value() const {
    const uint32_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    return (value_ & MASK) >> BIT_OFFSET;
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_field_value(uint32_t value) {
    const uint32_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    value_ = ((value << BIT_OFFSET) & MASK) | (value_ & ~MASK);
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_field_value_and_block(uint32_t value) {
    const uint32_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    value_ = ((value << BIT_OFFSET) & MASK) | (value_ & ~MASK);
    block_until_updated();
  }
};

}  // namespace tvsc::hal::boards
