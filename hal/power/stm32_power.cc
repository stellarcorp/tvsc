#include "hal/power/stm32_power.h"

namespace tvsc::hal::power {

void PowerStm32xxxx::enter_low_power_run_mode() {
  // Block until the low power regulator is ready.
  while (registers_->SR2.bit_field_value<1, 8>() == 0) {
    // Do nothing.
  }

  registers_->CR1.set_bit_field_value_and_block<1, 14>(1);

  // Block while we are in "main mode" where this flag reads a zero.
  // while (registers_->SR2.bit_field_value<1, 9>() == 0) {
  //   // Do nothing.
  // }
}

void PowerStm32xxxx::exit_low_power_run_mode() {
  registers_->CR1.set_bit_field_value_and_block<1, 14>(0);

  // Block while we are in "low power mode" where this flag reads a one.
  // while (registers_->SR2.bit_field_value<1, 9>() == 1) {
  //   // Do nothing.
  // }
}

}  // namespace tvsc::hal::power
