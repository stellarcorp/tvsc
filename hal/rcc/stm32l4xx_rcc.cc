#include "hal/rcc/stm32l4xx_rcc.h"

#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"

#ifdef __cplusplus
extern "C" {
#endif

volatile CTimeType current_time_us{0};

void SysTick_Handler() { current_time_us += 1e4; }

#ifdef __cplusplus
}
#endif

namespace tvsc::hal::rcc {

void RccStm32L4xx::enable_gpio_port(gpio::Port port) {
  rcc_registers_->AHB2ENR.set_bit_field_value_and_block<1>(1, static_cast<uint8_t>(port));
}

void RccStm32L4xx::disable_gpio_port(gpio::Port port) {
  rcc_registers_->AHB2ENR.set_bit_field_value_and_block<1>(0, static_cast<uint8_t>(port));
}

void RccStm32L4xx::set_clock_to_max_speed() {
  // Turn on HSI16 clock source.
  rcc_registers_->CR.set_bit_field_value<1, 8>(1);

  // Wait for HSI16 to be ready. The HSI16 ready flag is in bit 10 of the CR register.
  while (!rcc_registers_->CR.bit_field_value<1, 10>()) {
    // Do nothing.
  }

  // Configure system to use HSI16.
  rcc_registers_->CFGR.set_bit_field_value<2, 0>(0b01);

  // Wait for the system to acknowledge HSI16 is being used as core system clock.  The system
  // updates bits 2-3 of the CFGR register to indicate the current system clock.
  while (rcc_registers_->CFGR.bit_field_value<2, 2>() != 0b01) {
    // Do nothing.
  }

  // Turn off MSI clock source. MSI ON flag is in bit 0 of the CR register.
  rcc_registers_->CR.set_bit_field_value<1, 0>(0);

  // Update the SystemCoreClock value;
  SystemCoreClock = 16'000'000;

  // Update the SysTick configuration.
  update_sys_tick();
}

void RccStm32L4xx::set_clock_to_min_speed() {
  // Set the speed of the MSI clock. Bits 4-7 of the CR register configure the MSI clock. This exact
  // value sets the clock to 400 kHz.
  rcc_registers_->CR.set_bit_field_value<4, 4>(0b0010);

  // Weirdly, after setting the clock speed in a register, you have to actually write to another bit
  // to tell it to use that value.
  rcc_registers_->CR.set_bit_field_value<1, 3>(1);

  // Turn on MSI clock source. MSI ON flag is in bit 0 of the CR register.
  rcc_registers_->CR.set_bit_field_value<1, 0>(1);

  // Wait for it to be ready. The MSI ready flag is in bit 1 of the CR register.
  while (!rcc_registers_->CR.bit_field_value<1, 1>()) {
    // Do nothing.
  }

  // Configure system to use MSI.
  rcc_registers_->CFGR.set_bit_field_value<2, 0>(0b00);

  // Wait for the system to acknowledge MSI is being used as core system clock. The system
  // updates bits 2-3 of the CFGR register to indicate the current system clock.
  while (rcc_registers_->CFGR.bit_field_value<2, 2>() != 0b00) {
    // Do nothing.
  }

  // Turn off HSI clock source. The HSI on flag is in bit 8.
  rcc_registers_->CR.set_bit_field_value<1, 8>(0);

  // Update the SystemCoreClock value;
  SystemCoreClock = 400'000;

  // Update the SysTick configuration.
  update_sys_tick();
}

void RccStm32L4xx::update_sys_tick() { SysTick_Config(SystemCoreClock / 100); }

}  // namespace tvsc::hal::rcc