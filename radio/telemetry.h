#include "radio/settings.pb.h"

namespace tvsc::radio {

  class Telemetry final {
  private:
    tvsc_radio_TelemetryReport report_{};
    
  public:

    void increment_packets_received();
    void increment_packets_transmitted();

    void increment_packets_dropped();

    void increment_transmit_errors();

    void set_rssi(float rssi);

    /**
     * Set the power currently being consumed in Watts.
     */
    void set_power_usage_w(float power_w);

    const tvsc_radio_TelemetryReport& generate_telemetry_report();

      };

}
