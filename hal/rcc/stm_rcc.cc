#include "hal/rcc/stm_rcc.h"

#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"

#ifdef __cplusplus
extern "C" {
#endif

volatile CTimeType current_time_us{0};

void SysTick_Handler() { current_time_us += 1e2; }

#ifdef __cplusplus
}
#endif

namespace tvsc::hal::rcc {

void RccStm32H7xx::enable_gpio_port(gpio::Port port) {
  rcc_registers_->AHB4ENR.set_bit_field_value_and_block<1>(1, static_cast<uint8_t>(port));
}

void RccStm32H7xx::disable_gpio_port(gpio::Port port) {
  rcc_registers_->AHB4ENR.set_bit_field_value_and_block<1>(0, static_cast<uint8_t>(port));
}

void RccStm32H7xx::set_clock_to_max_speed() {
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

void RccStm32H7xx::set_clock_to_min_speed() {
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

void RccStm32H7xx::update_sys_tick() { SysTick_Config(SystemCoreClock / 10000); }

}  // namespace tvsc::hal::rcc
