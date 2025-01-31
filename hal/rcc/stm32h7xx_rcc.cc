#include "hal/rcc/stm32h7xx_rcc.h"

#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::rcc {

void RccStm32h7xx::enable_gpio_port_clock(gpio::Port port) {
  rcc_registers_->AHB4ENR.set_bit_field_value_and_block<1>(1, static_cast<uint8_t>(port));
}

void RccStm32h7xx::disable_gpio_port_clock(gpio::Port port) {
  rcc_registers_->AHB4ENR.set_bit_field_value_and_block<1>(0, static_cast<uint8_t>(port));
}

void RccStm32h7xx::enable_dac_clock() {
  rcc_registers_->APB1LENR.set_bit_field_value_and_block<1, 29>(1);
}

void RccStm32h7xx::disable_dac_clock() {
  rcc_registers_->APB1LENR.set_bit_field_value_and_block<1, 29>(0);
}

void RccStm32h7xx::enable_dma_clock() { __HAL_RCC_DMA1_CLK_ENABLE(); }

void RccStm32h7xx::disable_dma_clock() { __HAL_RCC_DMA1_CLK_DISABLE(); }

void RccStm32h7xx::enable_adc_clock() {
  // Use the system clock for the ADC.
  // rcc_registers_->D3CCIPR.set_bit_field_value_and_block<2, 16>(0b00);

  // Enable the clock on the ADC.
  rcc_registers_->AHB4ENR.set_bit_field_value_and_block<1, 24>(1);

  // Exit "deep-power-down" state.
  // adc_registers_->CR.set_bit_field_value_and_block<1, 29>(0);
  adc_registers_->CR.set_value_and_block(0);

  // Enable the ADC voltage regulator.
  adc_registers_->CR.set_bit_field_value_and_block<1, 28>(1);

  // Wait 20us or more. No API to check.
  {
    volatile uint32_t i = 2 * 20U * SystemCoreClock / 1'000'000;
    while (i > 0) {
      i--;
    }
  }

  // Clear the ADRDY flag.
  adc_registers_->ISR.set_bit_field_value<1, 0>(1);

  // Enable the ADC on the CR register.
  adc_registers_->CR.set_bit_field_value<1, 0>(1);

  // TODO(james): Determine if this is needed. Currently, it just hangs.
  // Wait for the ADRDY flag to be asserted.
  // while (!adc_registers_->ISR.bit_field_value<1, 0>()) {
  //   // Do nothing.
  // }

  // Clear the ADRDY flag for completeness.
  // adc_registers_->ISR.set_bit_field_value<1, 0>(1);
}

void RccStm32h7xx::disable_adc_clock() {
  // Just return if there is an ongoing ADDIS command to stop the ADC.
  if (adc_registers_->CR.bit_field_value<1, 1>()) {
    return;
  }

  // Stop any ongoing conversions.
  if (adc_registers_->CR.bit_field_value<1, 2>()) {
    adc_registers_->CR.set_bit_field_value<1, 0>(0);
  }

  // Issue the ADDIS disable command to the ADC.
  adc_registers_->CR.set_bit_field_value<1, 1>(1);
  // Monitor the ADEN bit to pause until the ADC has been disabled.
  while (adc_registers_->CR.bit_field_value<1, 0>()) {
    // Do nothing while the ADC shuts down.
  }

  // Enter "deep-power-down" state. Note that this automatically disables the voltage regulator as
  // well.
  adc_registers_->CR.set_bit_field_value_and_block<1, 29>(1);

  // Disable the clock for the ADC.
  rcc_registers_->AHB2ENR.set_bit_field_value_and_block<1, 13>(0);
}

void RccStm32h7xx::set_clock_to_max_speed() {
  // Set the speed of the HSI clock. Bits 3 and 4 of the CR register configure a clock divider for
  // the HSI clock. This exact value sets the clock to 32 MHz.
  rcc_registers_->CR.set_bit_field_value<2, 3>(0b01);

  // Turn on HSI clock source.
  rcc_registers_->CR.set_bit_field_value<1, 0>(1);

  // Wait HSI to be ready. The HSI ready flag is in bit 2 of the CR register. Also, we wait for the
  // divider to be ready. The HSI divider ready flag is in bit 5 of the CR register.
  while (!rcc_registers_->CR.bit_field_value<1, 2>() ||
         !rcc_registers_->CR.bit_field_value<1, 5>()) {
    // Do nothing.
  }

  // Configure system to use HSI.
  rcc_registers_->CFGR.set_bit_field_value<3, 0>(0b000);

  // Wait for the system to acknowledge HSI is being used as core system clock.  The system
  // updates bits 3-5 of the CFGR register to indicate the current system clock.
  while (rcc_registers_->CFGR.bit_field_value<3, 3>() != 0b000) {
    // Do nothing.
  }

  // Turn off CSI clock source. CSI ON flag is in bit 7 of the CR register.
  rcc_registers_->CR.set_bit_field_value<1, 7>(0);

  // Update the SystemCoreClock value;
  SystemCoreClock = 32'000'000;

  // Update the SysTick configuration.
  update_sys_tick();
}

void RccStm32h7xx::set_clock_to_min_speed() {
  // Turn on CSI clock source. Note that the speed of the CSI clock is fixed at 4 MHz, though it
  // might be configurable via various dividers. CSI ON flag is in bit 7 of the CR register.
  rcc_registers_->CR.set_bit_field_value<1, 7>(1);

  // Wait for it to be ready. The CSI ready flag is in bit 8 of the CR register.
  while (!rcc_registers_->CR.bit_field_value<1, 8>()) {
    // Do nothing.
  }

  // Configure system to use CSI.
  rcc_registers_->CFGR.set_bit_field_value<3, 0>(0b001);

  // Wait for the system to acknowledge CSI is being used as core system clock. The system
  // updates bits 3-5 of the CFGR register to indicate the current system clock.
  while (rcc_registers_->CFGR.bit_field_value<3, 3>() != 0b001) {
    // Do nothing.
  }

  // Turn off HSI clock source. The HSI on flag is in bit 0.
  rcc_registers_->CR.set_bit_field_value<1, 0>(0);

  // Update the SystemCoreClock value;
  SystemCoreClock = 4'000'000;

  // Update the SysTick configuration.
  update_sys_tick();
}

void RccStm32h7xx::update_sys_tick() { SysTick_Config(SystemCoreClock / 1000); }

}  // namespace tvsc::hal::rcc
