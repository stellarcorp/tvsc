#include "Arduino.h"
#include "third_party/arduino/blink_pin_mapping.h"

static const int LED_PIN{tvsc::third_party::arduino::BlinkPinMapping::led_pin()};

void setup() {
  Serial.begin(9600);

  Serial.println("Setting LED pin mode.");
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println("On...");
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  Serial.println("Off.");
  digitalWrite(LED_PIN, LOW);
  delay(500);
}
