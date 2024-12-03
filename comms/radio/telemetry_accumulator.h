#pragma once

#include <cstdint>
#include <string>

#include "comms/radio/transceiver_identification.h"

namespace tvsc::comms::radio {

class TelemetryAccumulator final {
 private:
  void update_time_measurement();
  void update_owned_metrics();

 public:
  TelemetryAccumulator(const TransceiverIdentification& device_id);

  void increment_fragments_received();
  void increment_fragments_transmitted();

  void increment_fragments_dropped();

  void increment_transmit_errors();

  void set_rssi_dbm(float rssi);

  void set_transmit_queue_size(uint32_t size);

  /**
   * Set the power currently being consumed in Watts.
   */
  void set_power_usage_w(float power_w);

  // TODO(james): Replace with method for generating a packet or fragment for transmission.
  std::string generate_report_string();
};

}  // namespace tvsc::comms::radio
