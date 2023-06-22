#include "radio/telemetry.h"

#include "radio/settings.pb.h"

namespace tvsc::radio {

    void Telemetry::increment_packets_received() {}
    void Telemetry::increment_packets_transmitted() {}

    void Telemetry::increment_packets_dropped() {}

    void Telemetry::increment_transmit_errors() {}

    void Telemetry::set_rssi(float rssi) {}

    /**
     * Set the power currently being consumed in Watts.
     */
  void Telemetry::set_power_usage_w(float power_w) {}

  const tvsc_radio_TelemetryReport& Telemetry::generate_telemetry_report() {
  return report_;
      }

}
