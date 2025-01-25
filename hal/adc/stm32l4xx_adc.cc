#include "hal/adc/stm32l4xx_adc.h"

#include "hal/adc/stm32l4xx_adc_register_bank.h"
#include "hal/gpio/gpio.h"
#include "hal/register.h"

extern uint32_t SystemCoreClock;

namespace tvsc::hal::adc {

static constexpr uint8_t get_channel(gpio::Port port, gpio::Pin pin) {
  if (port == 0) {
    if (pin == 0) {
      // ADC1_IN5
      return 5;
    } else if (pin == 1) {
      // ADC1_IN6
      return 6;
    } else if (pin == 2) {
      // ADC1_IN7
      return 7;
    } else if (pin == 3) {
      // ADC1_IN8
      return 8;
    } else if (pin == 4) {
      // ADC1_IN9
      return 9;
    } else if (pin == 5) {
      // ADC1_IN10
      return 10;
    } else if (pin == 6) {
      // ADC1_IN11
      return 11;
    } else if (pin == 7) {
      // ADC1_IN12
      return 12;
    }
  } else if (port == 1) {
  } else if (port == 2) {
  } else if (port == 3) {
  } else if (port == 4) {
  } else if (port == 5) {
  } else if (port == 6) {
  } else if (port == 7) {
  }
  return 0xff;
}

void AdcStm32l4xx::start_conversion(gpio::Port port, gpio::Pin pin, uint8_t /*gain*/) {
  // If the ADC is not enabled (that is, if ADEN != 1) then we enable the ADC.
  if (!registers_->CR.bit_field_value<1, 0>()) {
    // Clear the ADRDY flag.
    registers_->ISR.set_bit_field_value<1, 0>(1);

    // Enable the ADC by setting the ADEN flag on the CR register.
    registers_->CR.set_bit_field_value<1, 0>(1);

    // Wait for the ADRDY flag to be asserted.
    while (!registers_->ISR.bit_field_value<1, 0>()) {
      // Do nothing.
    }

    // Clear the ADRDY flag for completeness.
    registers_->ISR.set_bit_field_value<1, 0>(1);
  }

  /* TODO(james): Add error handling of some form. */
  const uint8_t channel{get_channel(port, pin)};
  if (channel == 0xff) {
    return;
  }

  // TODO(james): Implement gain by routing pin voltage through onboard opamp and doing the ADC
  // on the opamp's output.

  // Put the ADC in single conversion mode.
  registers_->CFGR.set_bit_field_value<1, 13>(0);

  // Turn off "discontinuous" mode as well. Not sure how "discontinuous" and single mode differ.
  registers_->CFGR.set_bit_field_value<1, 16>(0);

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

uint16_t AdcStm32l4xx::read_result() {
  return static_cast<uint16_t>(registers_->DR.bit_field_value<16, 0>());
}

bool AdcStm32l4xx::is_running() {
  return registers_->CR.bit_field_value<1, 2>() || registers_->CR.bit_field_value<1, 31>();
}

void AdcStm32l4xx::stop() {
  registers_->CR.set_bit_field_value<1, 4>(1);

  while (registers_->CR.bit_field_value<1, 4>()) {
    // ADC is stopping. Do nothing.
  }
}

void AdcStm32l4xx::calibrate_single_ended_input() {
  // Stop any ongoing conversions.
  if (registers_->CR.bit_field_value<1, 2>()) {
    registers_->CR.set_bit_field_value<1, 0>(0);
  }

  // Issue the ADDIS disable command to the ADC.
  registers_->CR.set_bit_field_value<1, 1>(1);
  // Monitor the ADEN bit to pause until the ADC has been disabled.
  while (registers_->CR.bit_field_value<1, 0>()) {
    // Do nothing while the ADC shuts down.
  }

  // Set up to calibrate single-ended conversions by setting the ADCALDIF to zero, meaning not
  // differential inputs mode.
  registers_->CR.set_bit_field_value<1, 30>(0);

  // Start the calibration.
  registers_->CR.set_bit_field_value<1, 31>(1);
}

void AdcStm32l4xx::calibrate_differential_input() {
  // Stop any ongoing conversions.
  if (registers_->CR.bit_field_value<1, 2>()) {
    registers_->CR.set_bit_field_value<1, 0>(0);
  }

  // Issue the ADDIS disable command to the ADC.
  registers_->CR.set_bit_field_value<1, 1>(1);
  // Monitor the ADEN bit to pause until the ADC has been disabled.
  while (registers_->CR.bit_field_value<1, 0>()) {
    // Do nothing while the ADC shuts down.
  }

  // Set up to calibrate single-ended conversions by setting the ADCALDIF to one.
  registers_->CR.set_bit_field_value<1, 30>(1);

  // Start the calibration.
  registers_->CR.set_bit_field_value<1, 31>(1);
}

uint16_t AdcStm32l4xx::read_calibration_factor() {
  uint16_t result{static_cast<uint16_t>(registers_->CALFACT.bit_field_value<7, 16>())};
  result = (result << 8) | static_cast<uint16_t>(registers_->CALFACT.bit_field_value<7, 0>());
  return result;
}

void AdcStm32l4xx::write_calibration_factor(uint16_t factor) {
  registers_->CALFACT.set_bit_field_value<7, 16>(factor >> 8);
  registers_->CALFACT.set_bit_field_value<7, 0>(factor);
}

}  // namespace tvsc::hal::adc
