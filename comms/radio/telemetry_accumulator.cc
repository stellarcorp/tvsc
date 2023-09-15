#include "comms/radio/telemetry_accumulator.h"

#include "comms/radio/nanopb_proto/settings.pb.h"
#include "comms/radio/settings.h"
#include "hal/time/time.h"

namespace tvsc::comms::radio {

void TelemetryAccumulator::update_time_measurement() {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_TIME)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(event.time_ms);
}

void TelemetryAccumulator::update_owned_metrics() {
  for (pb_size_t index = 0; index < report_.events_count; ++index) {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[index]};
    if (event.metric == tvsc_comms_radio_nano_Metric_FRAGMENTS_RX) {
      event.time_ms = tvsc::hal::time::time_millis();
    } else if (event.metric == tvsc_comms_radio_nano_Metric_FRAGMENTS_TX) {
      event.time_ms = tvsc::hal::time::time_millis();
    } else if (event.metric == tvsc_comms_radio_nano_Metric_DROPPED_FRAGMENTS) {
      event.time_ms = tvsc::hal::time::time_millis();
    } else if (event.metric == tvsc_comms_radio_nano_Metric_TX_ERRORS) {
      event.time_ms = tvsc::hal::time::time_millis();
    }
  }
}

TelemetryAccumulator::TelemetryAccumulator(uint32_t device_id) {
  // Initialize all of the metrics.

  size_t next_event_index{0};

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_TIME;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(event.time_ms);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_FRAGMENTS_RX;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_FRAGMENTS_TX;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_DROPPED_FRAGMENTS;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_TX_ERRORS;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_TX_QUEUE_SIZE;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(0);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_RSSI_DBM;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value(-127.f);
  }

  {
    tvsc_comms_radio_nano_TelemetryEvent& event{report_.events[next_event_index++]};
    event.device_id = device_id;
    event.time_ms = tvsc::hal::time::time_millis();
    event.metric = tvsc_comms_radio_nano_Metric_POWER_W;
    event.has_measurement = true;
    event.measurement = tvsc::comms::radio::as_discrete_value(0.f);
  }

  report_.events_count = next_event_index;
}

void TelemetryAccumulator::increment_fragments_received() {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_FRAGMENTS_RX)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement =
      tvsc::comms::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void TelemetryAccumulator::increment_fragments_transmitted() {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_FRAGMENTS_TX)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement =
      tvsc::comms::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void TelemetryAccumulator::increment_fragments_dropped() {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_DROPPED_FRAGMENTS)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement =
      tvsc::comms::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void TelemetryAccumulator::increment_transmit_errors() {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_TX_ERRORS)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement =
      tvsc::comms::radio::as_discrete_value<uint32_t>(as<uint32_t>(event.measurement) + 1);
}

void TelemetryAccumulator::set_rssi_dbm(float rssi_dbm) {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_RSSI_DBM)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::comms::radio::as_discrete_value(rssi_dbm);
}

void TelemetryAccumulator::set_transmit_queue_size(uint32_t size) {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_TX_QUEUE_SIZE)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::comms::radio::as_discrete_value<uint32_t>(size);
}

/**
 * Set the power currently being consumed in Watts.
 */
void TelemetryAccumulator::set_power_usage_w(float power_w) {
  tvsc_comms_radio_nano_TelemetryEvent& event{
      *find_event(report_, tvsc_comms_radio_nano_Metric_POWER_W)};
  event.time_ms = tvsc::hal::time::time_millis();
  event.measurement = tvsc::comms::radio::as_discrete_value(power_w);
}

const tvsc_comms_radio_nano_TelemetryReport& TelemetryAccumulator::generate_telemetry_report() {
  update_owned_metrics();
  update_time_measurement();
  return report_;
}

}  // namespace tvsc::comms::radio
