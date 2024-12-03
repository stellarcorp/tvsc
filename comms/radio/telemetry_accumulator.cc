#include "comms/radio/telemetry_accumulator.h"

#include "hal/time/time.h"

namespace tvsc::comms::radio {

void TelemetryAccumulator::update_time_measurement() {}

void TelemetryAccumulator::update_owned_metrics() {}

TelemetryAccumulator::TelemetryAccumulator(const TransceiverIdentification& /*device_id*/) {}

void TelemetryAccumulator::increment_fragments_received() {}

void TelemetryAccumulator::increment_fragments_transmitted() {}

void TelemetryAccumulator::increment_fragments_dropped() {}

void TelemetryAccumulator::increment_transmit_errors() {}

void TelemetryAccumulator::set_rssi_dbm(float rssi_dbm) {}

void TelemetryAccumulator::set_transmit_queue_size(uint32_t size) {}

void TelemetryAccumulator::set_power_usage_w(float power_w) {}

std::string TelemetryAccumulator::generate_report_string() { return ""; }

}  // namespace tvsc::comms::radio
