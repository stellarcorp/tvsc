#include "comms/radio/transceiver_identification.h"

#include "random/random.h"

namespace tvsc::comms::radio {

TransceiverIdentification TransceiverIdentification::initialize() {
  TransceiverIdentification result{};
  if (!load_transceiver_identification(result)) {
    result.expanded_id = tvsc::random::generate_random_value<uint64_t>();
    result.id = static_cast<uint8_t>(result.expanded_id & 0xff);
    save_transceiver_identification(result);
  }
  return result;
}

}  // namespace tvsc::comms::radio
