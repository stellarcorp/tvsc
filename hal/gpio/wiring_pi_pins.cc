#include "hal/gpio/pins.h"
#include "wiringPi.h"

namespace tvsc::hal::gpio {

void set_mode(uint8_t pin, PinMode mode) {
  switch (mode) {
    case PinMode::MODE_INPUT:
      pinMode(pin, INPUT);
      pullUpDnControl(pin, PUD_OFF);
      break;
    case PinMode::MODE_INPUT_PULLUP:
      pinMode(pin, INPUT);
      pullUpDnControl(pin, PUD_UP);
      break;
    case PinMode::MODE_INPUT_PULLDOWN:
      pinMode(pin, INPUT);
      pullUpDnControl(pin, PUD_DOWN);
      break;
    case PinMode::MODE_OUTPUT:
      pinMode(pin, OUTPUT);
      break;
  }
}

DigitalValue read_pin(uint8_t pin) {
  const int value{digitalRead(pin)};
  if (value == HIGH) {
    return DigitalValue::VALUE_HIGH;
  } else {
    return DigitalValue::VALUE_LOW;
  }
}

void write_pin(uint8_t pin, DigitalValue value) {
  switch (value) {
    case DigitalValue::VALUE_HIGH:
      digitalWrite(pin, HIGH);
      break;
    case DigitalValue::VALUE_LOW:
      digitalWrite(pin, LOW);
      break;
  }
}

}  // namespace tvsc::hal::gpio
