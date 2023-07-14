#include <string>

#include "hal/eeprom/eeprom.h"
#include "hal/gpio/pins.h"
#include "hal/output/output.h"
#include "radio/nanopb_proto/settings.pb.h"
#include "radio/packet.h"
#include "radio/rf69hcw.h"
#include "radio/rf69hcw_configuration.h"
#include "radio/transceiver.h"
#include "random/random.h"

int main() {
  tvsc::random::initialize_seed();
  tvsc::hal::gpio::initialize_gpio();

  static constexpr size_t RADIO_IDENTIFICATION_EEPROM_LOCATION{0};
  tvsc_radio_nano_RadioIdentification identification{};

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
    tvsc::radio::BestEffortTransceiver<tvsc::radio::RF69HCW, tvsc::radio::Packet,
                                       10 /* MAX_FRAGMENTS_PER_PACKET*/>
        radio{identification};
    radio.print_configuration();
    while (true) {
      radio.iterate();
    }
  } else {
    tvsc::radio::BestEffortTransceiver<tvsc::radio::RF69HCW, tvsc::radio::Packet,
                                       10 /* MAX_FRAGMENTS_PER_PACKET*/>
        radio{};
    radio.print_configuration();
    while (true) {
      radio.iterate();
    }
  }

  return 0;
}
