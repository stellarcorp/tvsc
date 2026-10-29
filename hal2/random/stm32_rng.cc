#include "hal2/random/stm32_rng.h"

#include <cstdint>

#include "hal2/config/facility.h"

namespace tvsc::hal2::random {

// Turn on power and clock to this peripheral.
void Stm32Rng::enable() noexcept {
  // Turn on the HSI48 oscillator.
  hsi48_driver_ = oscillator_->access();

  __HAL_RCC_RNG_CLK_ENABLE();

  hsi48_driver_.route_to(config::Facility::RNG_FACILITY, 0);

  // Enable the RNG peripheral by setting the RNGEN bit (bit 2) in the control register. All other
  // values are set to their reset value of zero.
  registers_->CR.set_bit_field_value<1, 2>(1);
}

void Stm32Rng::disable() noexcept {
  // Disable the RNG peripheral by clearing the RNGEN bit (bit 2) in the control register. All other
  // values are set to their reset value of zero.
  registers_->CR.set_value(0x00);

  __HAL_RCC_RNG_CLK_DISABLE();

  // Invalidate this activation instance, likely turning off the HSI48 oscillator.
  hsi48_driver_.invalidate();
}

[[nodiscard]] uint32_t Stm32Rng::generate() noexcept {
  while (true) {
    uint32_t sr = registers_->SR.value();

    // If data is ready, return the 32-bit value (reading DR automatically clears DRDY).
    if ((sr & 0x01) != 0) {
      return registers_->DR.value();
    }

    // Check for clock error (CEIS, bit 5) or seed error (SEIS, bit 6).
    if ((sr & 0x60) != 0) {
      // Clear the error flags by writing 0 to CEIS and SEIS in the status register.
      // Other bits in SR are read-only or cleared by hardware, so writing back
      // with bits 5 and 6 cleared safely acknowledges and resets the fault state.
      registers_->SR.set_bit_field_value<2, 5>(0);
    }
  }
}

}  // namespace tvsc::hal2::random
