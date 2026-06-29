#include "hal/led/hal_led.h"

#include "hal/gpio/gpio.h"
#include "hal/led/led.h"

namespace tvsc::hal::led {

void HalLed::enable() {
  led_.ensure_valid(led_peripheral_);
  led_.set_pin_mode(gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);
}

void HalLed::disable() {
  if (led_.is_valid()) {
    off();
    led_.set_pin_mode(gpio::PinMode::UNUSED);
    led_.invalidate();
  }
}

void HalLed::on() { return led_.write_pin(1); }
void HalLed::off() { return led_.write_pin(0); }
void HalLed::toggle() { return led_.toggle_pin(); }
bool HalLed::read() { return led_.read_pin(); }

}  // namespace tvsc::hal::led
