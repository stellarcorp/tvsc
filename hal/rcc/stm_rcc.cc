#include "hal/rcc/stm_rcc.h"

#include "hal/gpio/gpio.h"

namespace tvsc::hal::rcc {

void RccStm32H7xx::enable_gpio_port(gpio::Port port) {
  registers_->AHB4ENR.set_bit_field_value_and_block<1>(1, static_cast<uint8_t>(port));
}

void RccStm32H7xx::disable_gpio_port(gpio::Port port) {
  registers_->AHB4ENR.set_bit_field_value_and_block<1>(0, static_cast<uint8_t>(port));
}

}  // namespace tvsc::hal::rcc
