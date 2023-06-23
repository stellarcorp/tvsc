#include "radio/telemetry.h"

#include "hal/time/time.h"
#include "radio/settings.h"
#include "radio/settings.nanopb.pb.h"

namespace tvsc::radio {

void Telemetry::update_time_measurement() {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_TIME)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value<uint32_t>(event.time_ms);
}

void Telemetry::update_owned_measurements() {
  for (pb_size_t index = 0; index < report_.events_count; ++index) {
    tvsc_radio_TelemetryEvent& event{report_.events[index]};
    if (event.domain == tvsc_radio_Measurement_PACKETS_RX) {
      event.time_ms = tvsc::hal::time::time_millis();
    } else if (event.domain == tvsc_radio_Measurement_PACKETS_TX) {
      event.time_ms = tvsc::hal::time::time_millis();
    } else if (event.domain == tvsc_radio_Measurement_DROPPED_PACKETS) {
      event.time_ms = tvsc::hal::time::time_millis();
    } else if (event.domain == tvsc_radio_Measurement_TX_ERRORS) {
      event.time_ms = tvsc::hal::time::time_millis();
    }
  }
}

Telemetry::Telemetry() {
  // Initialize all of the measurements.

  size_t next_event_index{0};

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_TIME;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value<uint32_t>(event.time_ms);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_PACKETS_RX;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_PACKETS_TX;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_DROPPED_PACKETS;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_TX_ERRORS;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_TX_QUEUE_SIZE;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_RSSI_DBM;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value(-127.f);
  }

  {
    tvsc_radio_TelemetryEvent& event{report_.events[next_event_index++]};
    event.time_ms = tvsc::hal::time::time_millis();
    event.domain = tvsc_radio_Measurement_POWER_W;
    event.has_measurement = true;
    event.measurement = tvsc::radio::as_discrete_value(0.f);
  }

  report_.events_count = next_event_index;
}

void Telemetry::increment_packets_received() {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_PACKETS_RX)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void Telemetry::increment_packets_transmitted() {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_PACKETS_TX)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void Telemetry::increment_packets_dropped() {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_DROPPED_PACKETS)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void Telemetry::increment_transmit_errors() {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_TX_ERRORS)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void Telemetry::set_rssi_dbm(float rssi_dbm) {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_RSSI_DBM)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value(rssi_dbm);
}

void Telemetry::set_transmit_queue_size(uint32_t size) {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_TX_QUEUE_SIZE)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value<uint32_t>(size);
}

/**
 * Set the power currently being consumed in Watts.
 */
void Telemetry::set_power_usage_w(float power_w) {
  tvsc_radio_TelemetryEvent& event{*find_event(tvsc_radio_Measurement_POWER_W)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::radio::as_discrete_value(power_w);
}

const tvsc_radio_TelemetryReport& Telemetry::generate_telemetry_report() {
  update_owned_measurements();
  update_time_measurement();
  return report_;
}

}  // namespace tvsc::radio
