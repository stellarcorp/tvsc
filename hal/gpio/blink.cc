#include "Arduino.h"
#include "hal/gpio/blink_pin_mapping.h"
#include "hal/gpio/pins.h"
#include "hal/gpio/time.h"

static const int LED_PIN{tvsc::hal::gpio::BlinkPinMapping::led_pin()};

void setup() { tvsc::hal::gpio::set_mode(LED_PIN, tvsc::hal::gpio::PinMode::MODE_OUTPUT); }

void loop() {
  tvsc::hal::gpio::write_pin(LED_PIN, tvsc::hal::gpio::DigitalValue::VALUE_HIGH);
  tvsc::hal::gpio::delay_ms(200);
  tvsc::hal::gpio::write_pin(LED_PIN, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
  tvsc::hal::gpio::delay_ms(500);
}
