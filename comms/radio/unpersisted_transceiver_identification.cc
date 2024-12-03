#include "comms/radio/transceiver_identification.h"

namespace tvsc::comms::radio {

static constexpr size_t TRANSCEIVER_IDENTIFICATION_EEPROM_LOCATION{0};

bool TransceiverIdentification::load_transceiver_identification(TransceiverIdentification& /*id*/) {
  return false;
}

void TransceiverIdentification::save_transceiver_identification(
    const TransceiverIdentification& /*id*/) {
  // This set of platforms do not persist their transceiver identification.
}

}  // namespace tvsc::comms::radio
