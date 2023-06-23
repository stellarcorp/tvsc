#include "radio/settings.nanopb.pb.h"

namespace tvsc::radio {

class TelemetryAccumulator final {
 private:
  tvsc_radio_TelemetryReport report_{};

  void update_time_measurement();
  void update_owned_measurements();

  tvsc_radio_TelemetryEvent* find_event(tvsc_radio_Measurement domain) {
    for (pb_size_t index = 0; index < report_.events_count; ++index) {
      tvsc_radio_TelemetryEvent& event{report_.events[index]};
      if (event.domain == domain) {
        return &event;
      }
    }
    return nullptr;
  }

 public:
  TelemetryAccumulator();

  void increment_packets_received();
  void increment_packets_transmitted();

  void increment_packets_dropped();

  void increment_transmit_errors();

  void set_rssi_dbm(float rssi);

  void set_transmit_queue_size(uint32_t size);

  /**
   * Set the power currently being consumed in Watts.
   */
  void set_power_usage_w(float power_w);

  const tvsc_radio_TelemetryReport& generate_telemetry_report();
};

}  // namespace tvsc::radio
