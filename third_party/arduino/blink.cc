#include "Arduino.h"
#include "third_party/arduino/blink_pin_mapping.h"

static const int LED_PIN{tvsc::third_party::arduino::BlinkPinMapping::led_pin()};

void setup() { pinMode(LED_PIN, OUTPUT); }

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
