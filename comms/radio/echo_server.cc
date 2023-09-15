#include <string>

#include "comms/radio/fragment_transceiver.h"
#include "comms/radio/nanopb_proto/settings.pb.h"
#include "comms/radio/rf69hcw.h"
#include "comms/radio/rf69hcw_configuration.h"
#include "hal/eeprom/eeprom.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "random/random.h"

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  static constexpr size_t RADIO_IDENTIFICATION_EEPROM_LOCATION{0};
  tvsc_comms_radio_nano_RadioIdentification identification{};

  tvsc::hal::eeprom::Eeprom eeprom{};
  bool has_saved_identification{!eeprom.is_empty(RADIO_IDENTIFICATION_EEPROM_LOCATION)};
  if (has_saved_identification) {
    eeprom.get(RADIO_IDENTIFICATION_EEPROM_LOCATION, identification);
    tvsc::hal::output::println("Board identification initialized from EEPROM.");
  }

  if (!has_saved_identification) {
    eeprom.put(RADIO_IDENTIFICATION_EEPROM_LOCATION, identification);
    tvsc::hal::output::println("Board identification saved to EEPROM.");
  }

  if (has_saved_identification) {
    tvsc::comms::radio::FragmentTransceiver<tvsc::comms::radio::RF69HCW> radio{identification};
    radio.print_configuration();
    while (true) {
      radio.iterate();
    }
  } else {
    tvsc::comms::radio::FragmentTransceiver<tvsc::comms::radio::RF69HCW> radio{};
    radio.print_configuration();
    while (true) {
      radio.iterate();
    }
  }

  return 0;
}
