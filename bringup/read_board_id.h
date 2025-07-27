#include <chrono>
#include <cstdint>

#include "hal/adc/adc.h"
#include "hal/board_identification/board_ids.h"
#include "hal/gpio/gpio.h"

namespace tvsc::bringup {

/**
 * Blocking function to read the board id from the ID power and sense pins using an ADC.
 */
tvsc::hal::board_identification::BoardId read_board_id(
    tvsc::hal::gpio::GpioPeripheral& id_power_peripheral, tvsc::hal::gpio::Pin power_pin,
    tvsc::hal::gpio::GpioPeripheral& id_sense_peripheral, tvsc::hal::gpio::Pin sense_pin,
    tvsc::hal::adc::AdcPeripheral& adc_peripheral) {
  using namespace tvsc::hal::board_identification;
  using namespace tvsc::hal::gpio;

  // Turn on clocks for the peripherals that we want.
  auto id_power{id_power_peripheral.access()};
  auto id_sense{id_sense_peripheral.access()};
  auto adc{adc_peripheral.access()};

  adc.calibrate_single_ended_input();

  adc.set_resolution(BOARD_ID_ADC_RESOLUTION_BITS);
  if constexpr (BOARD_ID_ADC_RESOLUTION_BITS < 8) {
    adc.use_data_align_left();
  }

  id_power.set_pin_mode(power_pin, PinMode::OUTPUT_PUSH_PULL, PinSpeed::LOW);

  id_sense.set_pin_mode(sense_pin, PinMode::ANALOG);

  id_power.write_pin(power_pin, 1);

  const auto measured_value{adc.measure_value({id_sense.port(), sense_pin})};

  id_power.write_pin(power_pin, 0);

  return determine_board_id(measured_value);
}

}  // namespace tvsc::bringup
