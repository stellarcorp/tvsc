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

static constexpr uint32_t OUR_ID{1234};
static constexpr uint32_t OTHER_ID{5678};

PacketT build_packet(
    uint8_t sender_id = 1,
    tvsc::comms::radio::Protocol protocol = tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL,
    size_t payload_size = 0) {
  PacketT packet{};
  packet.set_protocol(protocol);
  packet.set_sender_id(sender_id);
  packet.set_payload_length(payload_size);
  return packet;
}

class PacketSniffer final : public tvsc::comms::route::PacketRouter<PacketT> {
 public:
  std::vector<PacketT> seen_packets{};

  bool route(const PacketT& packet) override {
    seen_packets.emplace_back(packet);
    return false;
  }
};

class TdmaTransceiverTest : public ::testing::Test {
 public:
  MockClock clock{};
  tvsc::comms::radio::MockRadio radio{clock};
  tvsc::comms::radio::TelemetryAccumulator telemetry{OUR_ID};
  TdmaTransceiver transceiver{radio, telemetry, clock};
  PacketSniffer packet_sniffer{};

  void SetUp() override {
    transceiver.set_id(OUR_ID);
    transceiver.add_router(packet_sniffer);
  }

  ::testing::AssertionResult allowed_to_transmit() {
    if (TimeSlot::Role::NODE_TX != transceiver.schedule().time_slot_role()) {
      return ::testing::AssertionFailure();
    }
    if (OUR_ID != transceiver.schedule().id()) {
      return ::testing::AssertionFailure();
    }
    if (OUR_ID != transceiver.schedule().time_slot_owner()) {
      return ::testing::AssertionFailure();
    }
    if (!transceiver.schedule().can_transmit()) {
      return ::testing::AssertionFailure();
    }
    return ::testing::AssertionSuccess();
  }

  ::testing::AssertionResult have_time_to_transmit() {
    if (transceiver.schedule().time_slot_duration_remaining_us() <
        radio.fragment_transmit_time_us()) {
      return ::testing::AssertionFailure();
    }
    return ::testing::AssertionSuccess();
  }

  void use_rx_only_frame() {
    FrameBuilder frame{};
    // Guard interval only.
    frame.add_guard_interval(100'000);
    transceiver.set_frame(frame.build());
  }

  void use_tx_rx_frame() {
    FrameBuilder frame{};

    // Allow us to transmit.
    frame.add_node_tx_slot(100'000, OUR_ID);

    // Provide for time skew.
    frame.add_guard_interval(10'000);

    // Allow another node to transmit.
    frame.add_node_tx_slot(100'000, OTHER_ID);

    // Provide for time skew.
    frame.add_guard_interval(10'000);

    transceiver.set_frame(frame.build());
  }

  void use_rx_tx_frame() {
    FrameBuilder frame{};

    // Allow another node to transmit.
    frame.add_node_tx_slot(100'000, OTHER_ID);

    // Provide for time skew.
    frame.add_guard_interval(10'000);

    // Allow us to transmit.
    frame.add_node_tx_slot(100'000, OUR_ID);

    // Provide for time skew.
    frame.add_guard_interval(10'000);

    transceiver.set_frame(frame.build());
  }
};

TEST_F(TdmaTransceiverTest, DoesNothingWithoutPackets) {
  use_tx_rx_frame();

  transceiver.iterate();
  clock.increment_current_time_micros();
  transceiver.iterate();

  EXPECT_EQ(0, packet_sniffer.seen_packets.size());
}

TEST_F(TdmaTransceiverTest, CanReceiveSinglePacket) {
  use_rx_only_frame();

  FragmentT fragment{};
  fragment.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
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

TEST_F(TdmaTransceiverTest, CanTxSinglePacket) {
  use_tx_rx_frame();

  const PacketT packet{build_packet()};
  transceiver.push_control_priority(packet);

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This should initiate the transmission.
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_tx_mode());

  // Now we will be halfway through the transmission.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
  transceiver.iterate();

  // We can still transmit.
  ASSERT_TRUE(allowed_to_transmit());
  // The packet hasn't been removed from the queue.
  ASSERT_EQ(1, transceiver.transmit_queue_size());
  // And we are still transmitting.
  EXPECT_TRUE(radio.in_tx_mode());

  // Now the transmission should be finished.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us());
  transceiver.iterate();

  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(1, radio.sent_fragments().size());
  EXPECT_EQ(packet.sender_id(), radio.sent_fragments().front().sender_id());
  EXPECT_EQ(packet.protocol(), radio.sent_fragments().front().protocol());
}

TEST_F(TdmaTransceiverTest, CanTxTwoPackets) {
  static constexpr size_t NUM_PACKETS{2};

  use_tx_rx_frame();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    transceiver.push_control_priority(build_packet(i));
  }

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(NUM_PACKETS, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This initiates the first transmission.
  transceiver.iterate();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    ASSERT_TRUE(allowed_to_transmit());
    ASSERT_TRUE(have_time_to_transmit());

    ASSERT_EQ(NUM_PACKETS - i, transceiver.transmit_queue_size());
    EXPECT_TRUE(radio.in_tx_mode());

    // Now we will be halfway through the transmission.
    clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
    transceiver.iterate();

    ASSERT_TRUE(allowed_to_transmit());

    ASSERT_EQ(NUM_PACKETS - i, transceiver.transmit_queue_size());
    EXPECT_TRUE(radio.in_tx_mode());

    // Now this transmission should be finished.
    // Note that we add one microsecond at the end to avoid issues with round-off errors.
    clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2 + 1);
    // This iterate() call will initiate the next transmission, if a fragment is available.
    transceiver.iterate();
  }

  // After the last packet, the radio should be out of TX mode, and the transmit queue should be
  // empty.
  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(NUM_PACKETS, radio.sent_fragments().size());
  for (size_t i = 0; i < radio.sent_fragments().size(); ++i) {
    EXPECT_EQ(i, radio.sent_fragments()[i].sender_id());
  }
}

TEST_F(TdmaTransceiverTest, CanTxManyPackets) {
  static constexpr size_t NUM_PACKETS{20};

  use_tx_rx_frame();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    transceiver.push_control_priority(build_packet(i));
  }

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(NUM_PACKETS, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This initiates the first transmission.
  transceiver.iterate();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    ASSERT_TRUE(allowed_to_transmit());
    ASSERT_TRUE(have_time_to_transmit());

    ASSERT_EQ(NUM_PACKETS - i, transceiver.transmit_queue_size());
    EXPECT_TRUE(radio.in_tx_mode());

    // Now we will be halfway through the transmission.
    clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
    transceiver.iterate();

    ASSERT_TRUE(allowed_to_transmit());

    ASSERT_EQ(NUM_PACKETS - i, transceiver.transmit_queue_size());
    EXPECT_TRUE(radio.in_tx_mode());

    // Now this transmission should be finished.
    // Note that we add one microsecond at the end to avoid issues with round-off errors.
    clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2 + 1);
    // This iterate() call will initiate the next transmission, if a fragment is available.
    transceiver.iterate();
  }

  // After the last packet, the radio should be out of TX mode, and the transmit queue should be
  // empty.
  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(NUM_PACKETS, radio.sent_fragments().size());
  for (size_t i = 0; i < radio.sent_fragments().size(); ++i) {
    EXPECT_EQ(i, radio.sent_fragments()[i].sender_id());
  }
}

TEST_F(TdmaTransceiverTest, CanTxPacketsRequiringTwoFragments) {
  static constexpr size_t NUM_FRAGMENTS_PER_PACKET{2};
  static constexpr size_t NUM_PACKETS{2};
  static constexpr size_t PAYLOAD_SIZE{MTU - PacketT::header_size() -
                                       PacketT::payload_size_bytes_required()};

  use_tx_rx_frame();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    const PacketT packet{build_packet(i, tvsc::comms::radio::Protocol::TVSC_TELEMETRY,
                                      NUM_FRAGMENTS_PER_PACKET * PAYLOAD_SIZE)};
    transceiver.push_control_priority(packet);
  }

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(NUM_PACKETS, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This initiates the first transmission.
  transceiver.iterate();

  for (size_t packet_index = 0; packet_index < NUM_PACKETS; ++packet_index) {
    for (size_t fragment_index = 0; fragment_index < NUM_FRAGMENTS_PER_PACKET; ++fragment_index) {
      ASSERT_TRUE(allowed_to_transmit());
      ASSERT_TRUE(have_time_to_transmit());

      ASSERT_EQ(NUM_PACKETS - packet_index, transceiver.transmit_queue_size());
      EXPECT_TRUE(radio.in_tx_mode());

      // Now we will be halfway through the transmission.
      clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
      transceiver.iterate();

      ASSERT_TRUE(allowed_to_transmit());

      ASSERT_EQ(NUM_PACKETS - packet_index, transceiver.transmit_queue_size());
      EXPECT_TRUE(radio.in_tx_mode());

      // Now this transmission should be finished.
      // Note that we add one microsecond at the end to avoid issues with round-off errors.
      clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2 + 1);
      // This iterate() call will initiate the next transmission, if a fragment is available.
      transceiver.iterate();
    }
  }

  // After the last packet, the radio should be out of TX mode, and the transmit queue should be
  // empty.
  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(NUM_PACKETS * NUM_FRAGMENTS_PER_PACKET, radio.sent_fragments().size());
  for (size_t i = 0; i < radio.sent_fragments().size(); ++i) {
    EXPECT_EQ(i / NUM_FRAGMENTS_PER_PACKET, radio.sent_fragments()[i].sender_id());
  }
}

TEST_F(TdmaTransceiverTest, CanTxPacketsRequiringMultipleFragments) {
  static constexpr size_t NUM_FRAGMENTS_PER_PACKET{4};
  static constexpr size_t NUM_PACKETS{2};
  static constexpr size_t PAYLOAD_SIZE{MTU - PacketT::header_size() -
                                       PacketT::payload_size_bytes_required()};

  use_tx_rx_frame();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    const PacketT packet{build_packet(i, tvsc::comms::radio::Protocol::TVSC_TELEMETRY,
                                      NUM_FRAGMENTS_PER_PACKET * PAYLOAD_SIZE)};
    transceiver.push_control_priority(packet);
  }

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(NUM_PACKETS, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This initiates the first transmission.
  transceiver.iterate();

  for (size_t packet_index = 0; packet_index < NUM_PACKETS; ++packet_index) {
    for (size_t fragment_index = 0; fragment_index < NUM_FRAGMENTS_PER_PACKET; ++fragment_index) {
      ASSERT_TRUE(allowed_to_transmit());
      ASSERT_TRUE(have_time_to_transmit());

      ASSERT_EQ(NUM_PACKETS - packet_index, transceiver.transmit_queue_size());
      EXPECT_TRUE(radio.in_tx_mode());

      // Now we will be halfway through the transmission.
      clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
      transceiver.iterate();

      ASSERT_TRUE(allowed_to_transmit());

      ASSERT_EQ(NUM_PACKETS - packet_index, transceiver.transmit_queue_size());
      EXPECT_TRUE(radio.in_tx_mode());

      // Now this transmission should be finished.
      // Note that we add one microsecond at the end to avoid issues with round-off errors.
      clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2 + 1);
      // This iterate() call will initiate the next transmission, if a fragment is available.
      transceiver.iterate();
    }
  }

  // After the last packet, the radio should be out of TX mode, and the transmit queue should be
  // empty.
  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(NUM_PACKETS * NUM_FRAGMENTS_PER_PACKET, radio.sent_fragments().size());
  for (size_t i = 0; i < radio.sent_fragments().size(); ++i) {
    EXPECT_EQ(i / NUM_FRAGMENTS_PER_PACKET, radio.sent_fragments()[i].sender_id());
  }
}

TEST_F(TdmaTransceiverTest, CanTxManyPacketsRequiringMultipleFragments) {
  static constexpr size_t NUM_FRAGMENTS_PER_PACKET{4};
  static constexpr size_t NUM_PACKETS{32};
  static constexpr size_t PAYLOAD_SIZE{MTU - PacketT::header_size() -
                                       PacketT::payload_size_bytes_required()};

  use_tx_rx_frame();

  for (size_t i = 0; i < NUM_PACKETS; ++i) {
    const PacketT packet{build_packet(i, tvsc::comms::radio::Protocol::TVSC_TELEMETRY,
                                      NUM_FRAGMENTS_PER_PACKET * PAYLOAD_SIZE)};
    transceiver.push_control_priority(packet);
  }

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(NUM_PACKETS, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This initiates the first transmission.
  transceiver.iterate();

  for (size_t packet_index = 0; packet_index < NUM_PACKETS; ++packet_index) {
    for (size_t fragment_index = 0; fragment_index < NUM_FRAGMENTS_PER_PACKET; ++fragment_index) {
      ASSERT_TRUE(allowed_to_transmit());
      ASSERT_TRUE(have_time_to_transmit());

      ASSERT_EQ(NUM_PACKETS - packet_index, transceiver.transmit_queue_size());
      EXPECT_TRUE(radio.in_tx_mode());

      // Now we will be halfway through the transmission.
      clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
      transceiver.iterate();

      ASSERT_TRUE(allowed_to_transmit());

      ASSERT_EQ(NUM_PACKETS - packet_index, transceiver.transmit_queue_size());
      EXPECT_TRUE(radio.in_tx_mode());

      // Now this transmission should be finished.
      // Note that we add one microsecond at the end to avoid issues with round-off errors.
      clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2 + 1);
      // This iterate() call will initiate the next transmission, if a fragment is available.
      transceiver.iterate();
    }
  }

  // After the last packet, the radio should be out of TX mode, and the transmit queue should be
  // empty.
  EXPECT_FALSE(radio.in_tx_mode());
  EXPECT_EQ(0, transceiver.transmit_queue_size());

  ASSERT_EQ(NUM_PACKETS * NUM_FRAGMENTS_PER_PACKET, radio.sent_fragments().size());
  for (size_t i = 0; i < radio.sent_fragments().size(); ++i) {
    EXPECT_EQ(i / NUM_FRAGMENTS_PER_PACKET, radio.sent_fragments()[i].sender_id());
  }
}

TEST_F(TdmaTransceiverTest, CanTxSinglePacketAtEndOfTxTimeSlot) {
  use_tx_rx_frame();

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  clock.increment_current_time_micros(transceiver.schedule().time_slot_duration_remaining_us() -
                                      radio.fragment_transmit_time_us() - 1);
  ASSERT_TRUE(have_time_to_transmit());

  const PacketT packet{build_packet()};
  transceiver.push_control_priority(packet);

  ASSERT_EQ(1, transceiver.transmit_queue_size());

  ASSERT_FALSE(radio.in_tx_mode());

  // This should initiate the transmission.
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_tx_mode());

  // Now we will be halfway through the transmission.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());

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

TEST_F(TdmaTransceiverTest, WontTxSinglePacketWithoutSufficientTime) {
  use_tx_rx_frame();

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  clock.increment_current_time_micros(transceiver.schedule().time_slot_duration_remaining_us() -
                                      radio.fragment_transmit_time_us() + 1);

  // There isn't enough time to TX in this time slot.
  ASSERT_FALSE(have_time_to_transmit());

  transceiver.push_control_priority(build_packet());

  ASSERT_EQ(1, transceiver.transmit_queue_size());

  ASSERT_FALSE(radio.in_tx_mode());

  // This should not initiate the transmission.
  transceiver.iterate();

  // This node is allowed to transmit.
  ASSERT_TRUE(allowed_to_transmit());

  // There are packets to transmit.
  ASSERT_EQ(1, transceiver.transmit_queue_size());

  // But there isn't enough time.
  EXPECT_FALSE(radio.in_tx_mode());
}

TEST_F(TdmaTransceiverTest, SingleFragmentPacketDequeuedAfterTransmitPastEndOfTimeSlot) {
  use_tx_rx_frame();

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  clock.increment_current_time_micros(transceiver.schedule().time_slot_duration_remaining_us() -
                                      radio.fragment_transmit_time_us() - 1);
  ASSERT_EQ(transceiver.schedule().time_slot_duration_remaining_us(),
            radio.fragment_transmit_time_us() + 1);

  transceiver.push_control_priority(build_packet());

  ASSERT_TRUE(allowed_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  ASSERT_FALSE(radio.in_tx_mode());

  // This should initiate the transmission.
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  ASSERT_TRUE(radio.in_tx_mode());

  // Now we will be halfway through the transmission.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  ASSERT_TRUE(radio.in_tx_mode());

  // We move the clock past the end of the time slot. We want to verify that the successfully
  // transmitted packet gets dequeued even if we don't get an iterate call until after the end of
  // the time slot.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us());
  transceiver.iterate();

  ASSERT_FALSE(allowed_to_transmit());
  ASSERT_FALSE(radio.in_tx_mode());

  EXPECT_EQ(0, transceiver.transmit_queue_size());
}

TEST_F(TdmaTransceiverTest, WontTransmitInRxSlot) {
  use_rx_only_frame();

  transceiver.push_control_priority(build_packet());

  ASSERT_FALSE(allowed_to_transmit());

  // We verify that we would still have time to TX, if we owned the time slot.
  ASSERT_GE(transceiver.schedule().time_slot_duration_remaining_us(),
            radio.fragment_transmit_time_us());
  // Here we verify that the time slot is not going to expire during the test.
  ASSERT_GE(transceiver.schedule().time_slot_duration_remaining_us(),
            2 * radio.fragment_transmit_time_us());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This should move us to RX mode.
  transceiver.iterate();

  ASSERT_FALSE(allowed_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_rx_mode());

  // If we could transmit, this would move us halfway into the TX time.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
  transceiver.iterate();

  ASSERT_FALSE(allowed_to_transmit());

  EXPECT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_rx_mode());

  // If we could transmit, the transmission would be complete now.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us());
  transceiver.iterate();

  EXPECT_TRUE(radio.in_rx_mode());
  EXPECT_EQ(1, transceiver.transmit_queue_size());
}

TEST_F(TdmaTransceiverTest, WillTransmitAfterTxSlotBegins) {
  use_rx_tx_frame();

  const PacketT packet{build_packet()};
  transceiver.push_control_priority(packet);

  ASSERT_FALSE(allowed_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_FALSE(radio.in_tx_mode());

  // This should move us to RX mode.
  transceiver.iterate();

  ASSERT_TRUE(radio.in_rx_mode());
  ASSERT_EQ(1, transceiver.transmit_queue_size());

  // Move forward in time until the next TX slot.
  while (!transceiver.schedule().can_transmit()) {
    clock.increment_current_time_micros(transceiver.schedule().time_slot_duration_remaining_us());
    transceiver.iterate();
  }

  // This should initiate the transmission.
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());
  ASSERT_TRUE(have_time_to_transmit());

  ASSERT_EQ(1, transceiver.transmit_queue_size());
  EXPECT_TRUE(radio.in_tx_mode());

  // Now we will be halfway through the transmission.
  clock.increment_current_time_micros(radio.fragment_transmit_time_us() / 2);
  transceiver.iterate();

  ASSERT_TRUE(allowed_to_transmit());

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
