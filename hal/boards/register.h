#pragma once

#include <cstdint>

namespace tvsc::hal::boards {

template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
constexpr uint8_t compute_mask() {
  return ((1U << NUM_BITS) - 1U) << BIT_OFFSET;
}

class Register final {
 private:
  // Note that a register's value is not initialized when the class is instantiated. If the value
  // was initialized, it would overwrite the value in the hardware register, including possibly
  // writing to private or undefined bits. Boards and register banks will need special logic to
  // bring all of the registers to valid initial states.
  volatile uint8_t value_;

 public:
  /**
   * Read the value of the register.
   */
  uint8_t value() const { return value_; }

  /**
   * Set the value of the register.
   */
  void set_value(uint8_t value) { value_ = value; }

  /**
   * Read the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  uint8_t field_value() const {
    const uint8_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    return (value_ & MASK) >> BIT_OFFSET;
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_field_value(uint8_t value) {
    const uint8_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    value_ = ((value << BIT_OFFSET) & MASK) | (value_ & ~MASK);
  }
};

}  // namespace tvsc::hal::boards
