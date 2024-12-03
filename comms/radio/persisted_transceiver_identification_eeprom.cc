#include "comms/radio/transceiver_identification.h"
#include "hal/eeprom/eeprom.h"

namespace tvsc::comms::radio {

static constexpr size_t TRANSCEIVER_IDENTIFICATION_EEPROM_LOCATION{0};

bool TransceiverIdentification::load_transceiver_identification(TransceiverIdentification& id) {
  tvsc::hal::eeprom::Eeprom eeprom{};
  bool has_saved_identification{!eeprom.is_empty(TRANSCEIVER_IDENTIFICATION_EEPROM_LOCATION)};
  if (has_saved_identification) {
    eeprom.get(TRANSCEIVER_IDENTIFICATION_EEPROM_LOCATION, id);
  }

  return has_saved_identification;
}

void TransceiverIdentification::save_transceiver_identification(
    const TransceiverIdentification& id) {
  tvsc::hal::eeprom::Eeprom eeprom{};
  eeprom.update(TRANSCEIVER_IDENTIFICATION_EEPROM_LOCATION, id);
}

}  // namespace tvsc::comms::radio
