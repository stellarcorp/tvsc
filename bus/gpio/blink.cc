#include "Arduino.h"
#include "bus/gpio/blink_pin_mapping.h"

static const int LED_PIN{tvsc::bus::gpio::BlinkPinMapping::led_pin()};

void setup() { pinMode(LED_PIN, OUTPUT); }

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
