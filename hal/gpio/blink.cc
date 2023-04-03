#include "Arduino.h"
#include "bus/gpio/blink_pin_mapping.h"
#include "bus/gpio/pins.h"
#include "bus/gpio/time.h"

static const int LED_PIN{tvsc::bus::gpio::BlinkPinMapping::led_pin()};

void setup() { tvsc::bus::gpio::set_mode(LED_PIN, tvsc::bus::gpio::PinMode::MODE_OUTPUT); }

void loop() {
  tvsc::bus::gpio::write_pin(LED_PIN, tvsc::bus::gpio::DigitalValue::VALUE_HIGH);
  tvsc::bus::gpio::delay_ms(200);
  tvsc::bus::gpio::write_pin(LED_PIN, tvsc::bus::gpio::DigitalValue::VALUE_LOW);
  tvsc::bus::gpio::delay_ms(500);
}
