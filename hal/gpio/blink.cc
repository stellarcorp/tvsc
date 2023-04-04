#include "hal/gpio/blink_pin_mapping.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "hal/time/time.h"

static const int LED_PIN{tvsc::hal::gpio::BlinkPinMapping::led_pin()};

int main() {
  tvsc::hal::output::println("Blink");
  tvsc::hal::gpio::set_mode(LED_PIN, tvsc::hal::gpio::PinMode::MODE_OUTPUT);

  while (true) {
    tvsc::hal::gpio::write_pin(LED_PIN, tvsc::hal::gpio::DigitalValue::VALUE_HIGH);
    tvsc::hal::time::delay_ms(200);
    tvsc::hal::gpio::write_pin(LED_PIN, tvsc::hal::gpio::DigitalValue::VALUE_LOW);
    tvsc::hal::time::delay_ms(500);
  }

  return 0;
}
