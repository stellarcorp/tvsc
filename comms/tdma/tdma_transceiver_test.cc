#include "comms/tdma/tdma_transceiver.h"

#include <cstdint>
#include <vector>

#include "comms/radio/fragment.h"
#include "comms/radio/mock_radio.h"
#include "comms/radio/protocol.h"
#include "gtest/gtest.h"
#include "hal/time/mock_clock.h"

namespace tvsc::comms::tdma {

static constexpr size_t MTU{tvsc::comms::radio::MockRadio::max_mtu()};
using tvsc::hal::time::MockClock;
using PacketT = tvsc::comms::packet::Packet;
using FragmentT = tvsc::comms::radio::Fragment<MTU>;
using TdmaTransceiver = TdmaTransceiverT<PacketT, MTU, 32, 127>;

static constexpr uint32_t ID{1234};

class PacketSniffer final : public tvsc::comms::route::PacketRouter<PacketT> {
 public:
  std::vector<PacketT> seen_packets{};

  bool route(const PacketT& packet) override {
    seen_packets.emplace_back(packet);
    return false;
  }
};

TEST(TdmaTransceiverTest, DoesNothingWithoutPackets) {
  MockClock clock{};
  tvsc::comms::radio::MockRadio radio{clock};
  TdmaSchedule schedule{clock, ID};
  tvsc::comms::radio::TelemetryAccumulator telemetry{ID};
  TdmaTransceiver transceiver{radio, schedule, telemetry, clock};
  PacketSniffer packet_sniffer{};
  transceiver.add_router(packet_sniffer);

  transceiver.iterate();

  clock.increment_current_time_micros();

  transceiver.iterate();

  EXPECT_EQ(0, packet_sniffer.seen_packets.size());
}

TEST(TdmaTransceiverTest, NodeCanReceiveSinglePacket) {
  MockClock clock{};
  tvsc::comms::radio::MockRadio radio{clock};
  TdmaSchedule schedule{clock, ID};
  tvsc::comms::radio::TelemetryAccumulator telemetry{ID};
  TdmaTransceiver transceiver{radio, schedule, telemetry, clock};
  PacketSniffer packet_sniffer{};
  transceiver.add_router(packet_sniffer);

  FragmentT fragment{};
  fragment.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  fragment.set_sender_id(1);
  // At 10us, this fragment will be available.
  radio.add_rx_fragment(10, fragment);

  transceiver.iterate();

  clock.increment_current_time_micros(5);
  ASSERT_FALSE(radio.has_fragment_available());
  transceiver.iterate();

  clock.increment_current_time_micros(5);
  ASSERT_TRUE(radio.has_fragment_available());
  transceiver.iterate();

  EXPECT_EQ(1, packet_sniffer.seen_packets.size());

  EXPECT_EQ(fragment.protocol(), packet_sniffer.seen_packets.front().protocol());
  EXPECT_EQ(fragment.sender_id(), packet_sniffer.seen_packets.front().sender_id());
}

TEST(TdmaTransceiverTest, NodeCanSendSinglePacket) {
  MockClock clock{};
  tvsc::comms::radio::MockRadio radio{clock};
  TdmaSchedule schedule{clock, ID};
  tvsc::comms::radio::TelemetryAccumulator telemetry{ID};
  TdmaTransceiver transceiver{radio, schedule, telemetry, clock};
  PacketSniffer packet_sniffer{};
  transceiver.add_router(packet_sniffer);

  schedule.set_frame(FrameBuilder::create_default_base_station_frame(ID));

  PacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  packet.set_sender_id(1);
  transceiver.push_control_priority(packet);

  ASSERT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());
  ASSERT_EQ(ID, schedule.id());
  ASSERT_EQ(ID, schedule.time_slot_owner());
  ASSERT_TRUE(schedule.can_transmit());
  ASSERT_GE(schedule.time_slot_duration_remaining_us(), radio.fragment_transmit_time_us());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This should initiate the transmission.
  transceiver.iterate();

  ASSERT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());
  ASSERT_EQ(ID, schedule.time_slot_owner());
  ASSERT_TRUE(schedule.can_transmit());
  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_tx_mode());

  // Now we will be halfway through the transmission.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
  transceiver.iterate();

  ASSERT_EQ(TimeSlot::Role::NODE_TX, schedule.time_slot_role());
  ASSERT_EQ(ID, schedule.time_slot_owner());
  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_tx_mode());

  // Now the transmission should be finished.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us());
  transceiver.iterate();

  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(1, radio.sent_fragments().size());
  EXPECT_EQ(packet.protocol(), radio.sent_fragments().front().protocol());
}

}  // namespace tvsc::comms::tdma
