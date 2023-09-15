#pragma once

#include "comms/radio/nanopb_proto/settings.pb.h"

namespace tvsc::comms::radio {

class TelemetryAccumulator final {
 private:
  tvsc_comms_radio_nano_TelemetryReport report_{};

  void update_time_measurement();
  void update_owned_metrics();

 public:
  static const tvsc_comms_radio_nano_TelemetryEvent* find_event(
      const tvsc_comms_radio_nano_TelemetryReport& report, tvsc_comms_radio_nano_Metric metric) {
    for (pb_size_t index = 0; index < report.events_count; ++index) {
      const tvsc_comms_radio_nano_TelemetryEvent& event{report.events[index]};
      if (event.metric == metric) {
        return &event;
      }
    }
    return nullptr;
  }

  static tvsc_comms_radio_nano_TelemetryEvent* find_event(tvsc_comms_radio_nano_TelemetryReport& report,
                                                    tvsc_comms_radio_nano_Metric metric) {
    for (pb_size_t index = 0; index < report.events_count; ++index) {
      tvsc_comms_radio_nano_TelemetryEvent& event{report.events[index]};
      if (event.metric == metric) {
        return &event;
      }
    }
    return nullptr;
  }

  TelemetryAccumulator(uint32_t device_id);

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

  const tvsc_comms_radio_nano_TelemetryReport& generate_telemetry_report();
};

}  // namespace tvsc::comms::radio
