#include "hal/adc/stm32l4xx_adc.h"

#include "hal/adc/stm32l4xx_adc_register_bank.h"
#include "hal/gpio/gpio.h"
#include "hal/register.h"

extern uint32_t SystemCoreClock;

namespace tvsc::hal::adc {

void AdcStm32L4xx::measure(gpio::Port port, gpio::Pin pin, uint8_t /*gain*/) {
  // Set clock to asynchronous mode. Should be the default on reset.
  // registers_->CCR.set_bit_field_value<2, 16>(0b00);

  // No prescaling of the clock. Should be the default on reset.
  // registers_->CCR.set_bit_field_value<4, 18>(0b0000);

  // Enable the ADC on the CR register.
  // registers_->CR.set_bit_field_value<1, 0>(1);

  /* TODO(james): Add a lookup table to find the channel from the requested pin. Here, we just
   * hardcode channel 9, which gives us an ADC input from the DAC output. */
  const uint8_t channel{9};

  // TODO(james): Implement gain by routing pin voltage through onboard opamp and doing the ADC
  // on the opamp's output.

  // Reset the CFGR register in order to disable everything that we don't want.
  // TODO(james): This should not be necessary.
  // registers_->CFGR.set_value(0x8000'0000);

  // Put the ADC in single conversion mode.
  registers_->CFGR.set_bit_field_value<1, 13>(0);

  // Turn off "discontinuous" mode as well. Not sure how "discontinuous" and single mode differ.
  registers_->CFGR.set_bit_field_value<1, 16>(0);

  // Put the ADC in "discontinuous" mode. Not sure how "discontinuous" and single mode differ.
  // registers_->CFGR.set_bit_field_value<1, 16>(1);

  // Right align the resulting data.
  registers_->CFGR.set_bit_field_value<1, 5>(0);

  // Set result data resolution to 8-bit.
  registers_->CFGR.set_bit_field_value<2, 3>(0b10);

  // Tell the ADC that we are doing one conversion.
  registers_->SQR1.set_bit_field_value<4, 0>(0b0000);

  // Configure the first sequence to use the pin as the input to the ADC.
  registers_->SQR1.set_bit_field_value<5, 6>(channel);

  // Configure the sample time to be ~50 ADC clock cycles.
  // TODO(james): Write code to configure the sample time in us instead of clock cycles of the ADC.
  if (channel < 10) {
    // Sample time configuration for the first nine channels are in SMPR1.
    registers_->SMPR1.set_bit_field_value<3>(0b100, 3 * channel);
  } else {
    // SMPR2 handles channels 10-18.
    registers_->SMPR2.set_bit_field_value<3>(0b100, 3 * (channel - 10));
  }

  // Select software triggering.
  registers_->CFGR.set_bit_field_value<2, 10>(0b00);

  // Start a conversion. This sets the ADSTART flag of the CR register.
  registers_->CR.set_bit_field_value<1, 2>(1);
}

uint16_t AdcStm32L4xx::read_result() {
  return static_cast<uint16_t>(registers_->DR.bit_field_value<16, 0>());
}

bool AdcStm32L4xx::is_running() { return registers_->CR.bit_field_value<1, 2>(); }

void AdcStm32L4xx::stop() {
  registers_->CR.set_bit_field_value<1, 4>(1);

  while (registers_->CR.bit_field_value<1, 4>()) {
    // ADC is stopping. Do nothing.
  }
}

}  // namespace tvsc::hal::adc
