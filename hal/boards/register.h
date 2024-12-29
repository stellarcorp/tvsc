#pragma once

#include <cstdint>

namespace tvsc::hal::boards {

template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
inline constexpr uint32_t compute_mask() {
  static_assert(NUM_BITS + BIT_OFFSET <= 32,
                "Invalid mask parameters. The total number of bits (offset and the number "
                "of bits in the mask) must be less than or equal to the size of the word "
                "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                "template parameters to call this method.");
  return ((1UL << NUM_BITS) - 1UL) << BIT_OFFSET;
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
  void block_until_updated() const volatile {
    uint32_t temp = value_;
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
  uint32_t field_value() const volatile {
    static_assert(NUM_BITS + BIT_OFFSET <= 32,
                  "Invalid bit field parameters. The total number of bits (offset and the number "
                  "of bits in the field) must be less than or equal to the size of the register "
                  "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                  "template parameters to call this method.");
    static constexpr uint32_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    return (value_ & MASK) >> BIT_OFFSET;
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_field_value(uint32_t value) volatile {
    static_assert(NUM_BITS + BIT_OFFSET <= 32,
                  "Invalid bit field parameters. The total number of bits (offset and the number "
                  "of bits in the field) must be less than or equal to the size of the register "
                  "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                  "template parameters to call this method.");
    static constexpr uint32_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    value_ = ((value << BIT_OFFSET) & MASK) | (value_ & ~MASK);
  }

  /**
   * Set the value of a bit field in a register.
   */
  template <uint8_t NUM_BITS, uint8_t BIT_OFFSET>
  void set_field_value_and_block(uint32_t value) volatile {
    static_assert(NUM_BITS + BIT_OFFSET <= 32,
                  "Invalid bit field parameters. The total number of bits (offset and the number "
                  "of bits in the field) must be less than or equal to the size of the register "
                  "which is 32 bits. Likely, this is a typo or other scrivener's error in the "
                  "template parameters to call this method.");
    static constexpr uint32_t MASK{compute_mask<NUM_BITS, BIT_OFFSET>()};
    value_ = ((value << BIT_OFFSET) & MASK) | (value_ & ~MASK);
    block_until_updated();
  }
};

}  // namespace tvsc::hal::boards
