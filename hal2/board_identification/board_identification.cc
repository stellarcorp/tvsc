#include "hal2/board_identification/board_identification.h"

#include "base/enums.h"
#include "hal2/board_identification/board_ids.h"
#include "hal2/clock/clock.h"
#include "hal2/gpio/gpio.h"

namespace tvsc::hal2::board_identification {

void BoardIdentificationDevice::enable() noexcept {
  power_pin_.ensure_valid(power_pin_device_);
  power_pin_.set_pin_mode(gpio::PinMode::OUTPUT_PUSH_PULL, gpio::PinSpeed::LOW);

  sense_pin_.ensure_valid(sense_pin_device_);
  sense_pin_.set_pin_mode(gpio::PinMode::ANALOG);

  adc_.ensure_valid(*adc_device_);
}

void BoardIdentificationDevice::disable() noexcept {
  adc_.invalidate();

  // Reset the sense pin.
  sense_pin_.invalidate();

  // Reset the power pin.
  power_pin_.off();
  power_pin_.set_pin_mode(gpio::PinMode::ANALOG);
  power_pin_.invalidate();
}

void BoardIdentificationDevice::start() noexcept {}
[[nodiscard]] bool BoardIdentificationDevice::is_running() noexcept { return false; }
void BoardIdentificationDevice::stop() noexcept {}

}  // namespace tvsc::hal2::board_identification
