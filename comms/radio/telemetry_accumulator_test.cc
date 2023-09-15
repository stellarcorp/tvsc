#include "comms/radio/telemetry_accumulator.h"

#include "gtest/gtest.h"
#include "comms/radio/nanopb_proto/settings.pb.h"
#include "comms/radio/settings.h"

namespace tvsc::comms::radio {

static constexpr uint32_t DEVICE_ID{0};

TEST(TelemetryAccumulatorTest, ReportOnInitializedIncludesTimeEvent) {
  TelemetryAccumulator telemetry{DEVICE_ID};
  const tvsc_comms_radio_nano_TelemetryReport& report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* time_event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_TIME)};
  ASSERT_NE(nullptr, time_event);
  EXPECT_GT(time_event->time_ms, 0);
  EXPECT_GT(as<uint32_t>(time_event->measurement), 0);
  EXPECT_EQ(as<uint32_t>(time_event->measurement), time_event->time_ms);
}

TEST(TelemetryAccumulatorTest, ReportOnInitializedIncludesAllOwnedMetrics) {
  TelemetryAccumulator telemetry{DEVICE_ID};
  const tvsc_comms_radio_nano_TelemetryReport& report{telemetry.generate_telemetry_report()};

  for (auto metric : {tvsc_comms_radio_nano_Metric_PACKETS_RX, tvsc_comms_radio_nano_Metric_PACKETS_TX,
                      tvsc_comms_radio_nano_Metric_DROPPED_PACKETS, tvsc_comms_radio_nano_Metric_TX_ERRORS}) {
    const tvsc_comms_radio_nano_TelemetryEvent* event{TelemetryAccumulator::find_event(report, metric)};
    ASSERT_NE(nullptr, event);
    EXPECT_EQ(0, as<uint32_t>(event->measurement));
  }
}

TEST(TelemetryAccumulatorTest, CanIncrementPacketsReceived) {
  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.increment_packets_received();

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_PACKETS_RX)};

  EXPECT_EQ(1, as<uint32_t>(event->measurement));
}

TEST(TelemetryAccumulatorTest, CanIncrementPacketsTransmitted) {
  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.increment_packets_transmitted();

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_PACKETS_TX)};

  EXPECT_EQ(1, as<uint32_t>(event->measurement));
}

TEST(TelemetryAccumulatorTest, CanIncrementPacketsDropped) {
  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.increment_packets_dropped();

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_DROPPED_PACKETS)};

  EXPECT_EQ(1, as<uint32_t>(event->measurement));
}

TEST(TelemetryAccumulatorTest, CanIncrementTransmitErrors) {
  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.increment_transmit_errors();

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_TX_ERRORS)};

  EXPECT_EQ(1, as<uint32_t>(event->measurement));
}

TEST(TelemetryAccumulatorTest, CanReportPower) {
  static constexpr float POWER_W{77.7};

  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.set_power_usage_w(POWER_W);

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_POWER_W)};

  EXPECT_EQ(POWER_W, as<float>(event->measurement));
}

TEST(TelemetryAccumulatorTest, CanReportRssi) {
  static constexpr float RSSI_DBM{-81.5};

  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.set_rssi_dbm(RSSI_DBM);

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_RSSI_DBM)};

  EXPECT_EQ(RSSI_DBM, as<float>(event->measurement));
}

TEST(TelemetryAccumulatorTest, CanReportTxQueueSize) {
  static constexpr uint32_t QUEUE_SIZE{11};

  TelemetryAccumulator telemetry{DEVICE_ID};

  telemetry.set_transmit_queue_size(QUEUE_SIZE);

  const tvsc_comms_radio_nano_TelemetryReport report{telemetry.generate_telemetry_report()};
  const tvsc_comms_radio_nano_TelemetryEvent* event{
      TelemetryAccumulator::find_event(report, tvsc_comms_radio_nano_Metric_TX_QUEUE_SIZE)};

  EXPECT_EQ(QUEUE_SIZE, as<uint32_t>(event->measurement));
}

}  // namespace tvsc::comms::radio
