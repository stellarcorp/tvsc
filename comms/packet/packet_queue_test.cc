#include "comms/packet/packet_queue.h"

#include <stdexcept>

#include "comms/packet/packet.h"
#include "comms/radio/fragment.h"
#include "comms/radio/protocol.h"
#include "gmock/gmock.h"

namespace tvsc::comms::packet {

using TypicalPacketT = PacketT<64>;
using JumboPacketT = PacketT<1500>;

TEST(PacketTxQueueTest, CanAddPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_normal_priority(packet);

  EXPECT_FALSE(queue.empty());
}

TEST(PacketTxQueueTest, ReturnsSingleImmediatePacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_immediate_priority(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsSingleControlPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_control_priority(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsSingleNormalPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_normal_priority(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsSingleLowPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_low_priority(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsImmediatePacketEvenIfOtherPacketsAvailable) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT immediate_packet{};
  immediate_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  immediate_packet.set_sender_id(42);

  TypicalPacketT control_packet{};
  control_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  control_packet.set_sender_id(43);

  TypicalPacketT normal_packet{};
  normal_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  normal_packet.set_sender_id(44);

  TypicalPacketT low_packet{};
  low_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  low_packet.set_sender_id(45);

  queue.push_immediate_priority(immediate_packet);
  queue.push_control_priority(control_packet);
  queue.push_normal_priority(normal_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(immediate_packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailable) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT control_packet{};
  control_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  control_packet.set_sender_id(43);

  TypicalPacketT normal_packet{};
  normal_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  normal_packet.set_sender_id(44);

  TypicalPacketT low_packet{};
  low_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  low_packet.set_sender_id(45);

  queue.push_control_priority(control_packet);
  queue.push_normal_priority(normal_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == control_packet || packet == normal_packet || packet == low_packet);
}

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingControl) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT normal_packet{};
  normal_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  normal_packet.set_sender_id(44);

  TypicalPacketT low_packet{};
  low_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  low_packet.set_sender_id(45);

  queue.push_normal_priority(normal_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == normal_packet || packet == low_packet);
}

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingNormal) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT control_packet{};
  control_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  control_packet.set_sender_id(43);

  TypicalPacketT low_packet{};
  low_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  low_packet.set_sender_id(45);

  queue.push_control_priority(control_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == control_packet || packet == low_packet);
}

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingLow) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT control_packet{};
  control_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  control_packet.set_sender_id(43);

  TypicalPacketT normal_packet{};
  normal_packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  normal_packet.set_sender_id(44);

  queue.push_control_priority(control_packet);
  queue.push_normal_priority(normal_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == control_packet || packet == normal_packet);
}

TEST(FragmentSinkTest, EmptyQueueHasNoFragments) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, sizeof(TypicalPacketT), 8> sink{
      queue.create_fragment_sink<sizeof(TypicalPacketT), 8>()};

  ASSERT_TRUE(sink.empty());
  EXPECT_FALSE(sink.has_more_fragments());
  EXPECT_THROW(sink.encode_next_packet(), std::logic_error);
}

TEST(FragmentSinkTest, GivesSingleFragmentFromSmallPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, sizeof(TypicalPacketT), 8> sink{
      queue.create_fragment_sink<sizeof(TypicalPacketT), 8>()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_immediate_priority(packet);

  ASSERT_FALSE(sink.empty());

  ASSERT_TRUE(sink.encode_next_packet());

  EXPECT_TRUE(sink.has_more_fragments());

  int fragment_count{0};
  while (sink.has_more_fragments()) {
    ++fragment_count;
    sink.next_fragment();
  }

  EXPECT_EQ(1, fragment_count);
}

TEST(FragmentSinkTest, DesiredIterationMethodWorks) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, sizeof(TypicalPacketT), 8> sink{
      queue.create_fragment_sink<sizeof(TypicalPacketT), 8>()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_immediate_priority(packet);

  int fragment_count{0};
  int packet_count{0};
  while (!sink.empty()) {
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      ++fragment_count;
    }
    sink.pop_packet();
    ++packet_count;
  }

  EXPECT_EQ(1, fragment_count);
  EXPECT_EQ(1, packet_count);
}

TEST(FragmentSinkTest, HandlesTwoPackets) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, sizeof(TypicalPacketT), 8> sink{
      queue.create_fragment_sink<sizeof(TypicalPacketT), 8>()};

  TypicalPacketT packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
  packet.set_sender_id(42);

  queue.push_immediate_priority(packet);

  packet.set_sender_id(43);
  queue.push_normal_priority(packet);

  int fragment_count{0};
  int packet_count{0};
  while (!sink.empty()) {
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      ++fragment_count;
    }
    sink.pop_packet();
    ++packet_count;
  }

  EXPECT_EQ(2, fragment_count);
  EXPECT_EQ(2, packet_count);
}

TEST(FragmentSinkTest, HandlesManyPackets) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, sizeof(TypicalPacketT), 8> sink{
      queue.create_fragment_sink<sizeof(TypicalPacketT), 8>()};

  static constexpr int NUM_PACKETS{32};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    TypicalPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(42 + i);

    queue.push_normal_priority(packet);
  }

  int packet_count{0};
  int fragment_count{0};
  while (!sink.empty()) {
    LOG(INFO) << "packet_count: " << packet_count << ", fragment_count: " << fragment_count
              << ", queue.elements_available(): " << queue.elements_available();
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      ++fragment_count;
    }
    sink.pop_packet();
    ++packet_count;
  }

  EXPECT_EQ(NUM_PACKETS, packet_count);
  EXPECT_EQ(NUM_PACKETS, fragment_count);
}

TEST(FragmentSinkTest, HandlesPacketsRequiringTwoFragments) {
  using FragmentT = tvsc::comms::radio::Fragment<40>;
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, FragmentT::mtu(), 8> sink{
      queue.create_fragment_sink<FragmentT::mtu(), 8>()};

  // This requires two fragments, since we need to encode the header and the payload. The payload
  // alone takes an MTU; with the header, we need at least two fragments to encode. See
  // static_asserts on the next few lines.
  static constexpr size_t PAYLOAD_SIZE{sink.mtu()};

  static_assert(TypicalPacketT::max_payload_size() > PAYLOAD_SIZE);
  static_assert(TypicalPacketT::header_size() + PAYLOAD_SIZE > FragmentT::mtu());
  static_assert(2 * TypicalPacketT::header_size() + PAYLOAD_SIZE <= 2 * FragmentT::mtu());

  static constexpr int NUM_PACKETS{1};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    TypicalPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(42 + i);

    packet.set_payload_length(PAYLOAD_SIZE);

    queue.push_normal_priority(packet);
  }

  int fragment_count{0};
  int packet_count{0};
  while (!sink.empty()) {
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      ++fragment_count;
    }
    sink.pop_packet();
    ++packet_count;
  }

  EXPECT_EQ(2 * NUM_PACKETS, fragment_count);
  EXPECT_EQ(NUM_PACKETS, packet_count);
}

TEST(FragmentSinkTest, HandlesPacketsRequiringThreeFragments) {
  using FragmentT = tvsc::comms::radio::Fragment<30>;
  PacketTxQueue<TypicalPacketT, 128> queue{};
  FragmentSink<TypicalPacketT, 128, FragmentT::mtu(), 8> sink{
      queue.create_fragment_sink<FragmentT::mtu(), 8>()};

  // This requires two fragments, since we need to encode the header and the payload. The payload
  // alone takes an MTU; with the header, we need at least two fragments to encode. See
  // static_asserts on the next few lines.
  static constexpr size_t PAYLOAD_SIZE{2 * sink.mtu()};

  static_assert(TypicalPacketT::max_payload_size() > PAYLOAD_SIZE);
  static_assert(TypicalPacketT::header_size() + PAYLOAD_SIZE > FragmentT::mtu());
  static_assert(2 * TypicalPacketT::header_size() + PAYLOAD_SIZE > 2 * FragmentT::mtu());
  static_assert(3 * TypicalPacketT::header_size() + PAYLOAD_SIZE <= 3 * FragmentT::mtu());

  static constexpr int NUM_PACKETS{1};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    TypicalPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(42 + i);

    packet.set_payload_length(PAYLOAD_SIZE);

    queue.push_normal_priority(packet);
  }

  int fragment_count{0};
  int packet_count{0};
  while (!sink.empty()) {
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      ++fragment_count;
    }
    sink.pop_packet();
    ++packet_count;
  }

  EXPECT_EQ(3 * NUM_PACKETS, fragment_count);
  EXPECT_EQ(NUM_PACKETS, packet_count);
}

TEST(FragmentSinkTest, HandlesPacketsRequiringManyFragments) {
  using FragmentT = tvsc::comms::radio::Fragment<64>;
  PacketTxQueue<JumboPacketT, 128> queue{};
  FragmentSink<JumboPacketT, 128, FragmentT::mtu(), 100> sink{
      queue.create_fragment_sink<FragmentT::mtu(), 100>()};

  static constexpr size_t PAYLOAD_SIZE{1250};
  static constexpr size_t EXPECTED_FRAGMENT_COUNT{22};

  // Assert that we have the correct values set for this test.
  static_assert(JumboPacketT::max_payload_size() > PAYLOAD_SIZE);
  static_assert(JumboPacketT::header_size() + PAYLOAD_SIZE > FragmentT::mtu());
  static_assert(EXPECTED_FRAGMENT_COUNT * JumboPacketT::header_size() + PAYLOAD_SIZE <=
                EXPECTED_FRAGMENT_COUNT * FragmentT::mtu());

  static constexpr int NUM_PACKETS{1};
  for (int i = 0; i < NUM_PACKETS; ++i) {
    JumboPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(42 + i);

    packet.set_payload_length(PAYLOAD_SIZE);

    queue.push_normal_priority(packet);
  }

  int fragment_count{0};
  int packet_count{0};
  size_t payload_size_from_fragments{0};
  while (!sink.empty()) {
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      const FragmentT& fragment = sink.fragment();
      payload_size_from_fragments += fragment.payload_size();
      ++fragment_count;
    }
    sink.pop_packet();
    ++packet_count;
  }

  EXPECT_EQ(PAYLOAD_SIZE, payload_size_from_fragments);
  EXPECT_EQ(EXPECTED_FRAGMENT_COUNT * NUM_PACKETS, fragment_count);
  EXPECT_EQ(NUM_PACKETS, packet_count);
}

TEST(FragmentSinkTest, ManyMultifragmentPackets) {
  static constexpr size_t MTU{64};
  static constexpr size_t NUM_PACKETS_PER_QUEUE{32};
  static constexpr size_t MAX_FRAGMENTS_PER_PACKET{32};

  using FragmentT = tvsc::comms::radio::Fragment<MTU>;

  PacketTxQueue<JumboPacketT, NUM_PACKETS_PER_QUEUE> queue{};
  FragmentSink<JumboPacketT, NUM_PACKETS_PER_QUEUE, FragmentT::mtu(), MAX_FRAGMENTS_PER_PACKET>
      sink{queue.create_fragment_sink<FragmentT::mtu(), MAX_FRAGMENTS_PER_PACKET>()};

  static constexpr size_t EXPECTED_FRAGMENT_COUNT{20};
  static constexpr size_t PAYLOAD_SIZE{
      (MTU - JumboPacketT::header_size() - JumboPacketT::payload_size_bytes_required()) *
      EXPECTED_FRAGMENT_COUNT};

  // Assert that we have the correct values set for this test.
  static_assert(JumboPacketT::max_payload_size() >= PAYLOAD_SIZE);
  static_assert(EXPECTED_FRAGMENT_COUNT <= MAX_FRAGMENTS_PER_PACKET);
  static_assert(JumboPacketT::header_size() + PAYLOAD_SIZE > FragmentT::mtu());
  static_assert(EXPECTED_FRAGMENT_COUNT * JumboPacketT::header_size() + PAYLOAD_SIZE <=
                EXPECTED_FRAGMENT_COUNT * FragmentT::mtu());

  for (size_t i = 0; i < NUM_PACKETS_PER_QUEUE; ++i) {
    JumboPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(static_cast<uint8_t>(i));

    packet.set_payload_length(PAYLOAD_SIZE);

    queue.push_immediate_priority(packet);
    queue.push_control_priority(packet);
    queue.push_normal_priority(packet);
    queue.push_low_priority(packet);
  }

  int packet_count{0};
  while (!sink.empty()) {
    int fragment_count{0};
    size_t payload_size_from_fragments{0};
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      const FragmentT& fragment = sink.fragment();
      payload_size_from_fragments += fragment.payload_size();
      ++fragment_count;
    }
    EXPECT_EQ(PAYLOAD_SIZE, payload_size_from_fragments);

    sink.pop_packet();
    ++packet_count;

    EXPECT_EQ(EXPECTED_FRAGMENT_COUNT, fragment_count);
  }

  EXPECT_EQ(4 * NUM_PACKETS_PER_QUEUE, packet_count);
}

TEST(FragmentSinkTest, NearExtremes) {
  static constexpr size_t MTU{16};
  static constexpr size_t NUM_PACKETS_PER_QUEUE{1024};
  static constexpr size_t MAX_FRAGMENTS_PER_PACKET{127};

  using FragmentT = tvsc::comms::radio::Fragment<MTU>;

  PacketTxQueue<JumboPacketT, NUM_PACKETS_PER_QUEUE> queue{};
  FragmentSink<JumboPacketT, NUM_PACKETS_PER_QUEUE, FragmentT::mtu(), MAX_FRAGMENTS_PER_PACKET>
      sink{queue.create_fragment_sink<FragmentT::mtu(), MAX_FRAGMENTS_PER_PACKET>()};

  static constexpr size_t EXPECTED_FRAGMENT_COUNT{MAX_FRAGMENTS_PER_PACKET};
  static constexpr size_t PAYLOAD_SIZE{FragmentT::max_payload_size() * EXPECTED_FRAGMENT_COUNT};

  // Assert that we have the correct values set for this test.
  static_assert(JumboPacketT::max_payload_size() >= PAYLOAD_SIZE);
  static_assert(EXPECTED_FRAGMENT_COUNT <= MAX_FRAGMENTS_PER_PACKET);

  static_assert(JumboPacketT::header_size() + JumboPacketT::payload_size_bytes_required() +
                    PAYLOAD_SIZE >
                FragmentT::mtu());

  for (size_t i = 0; i < NUM_PACKETS_PER_QUEUE; ++i) {
    JumboPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(static_cast<uint8_t>(i));

    packet.set_payload_length(PAYLOAD_SIZE);

    queue.push_immediate_priority(packet);
    queue.push_control_priority(packet);
    queue.push_normal_priority(packet);
    queue.push_low_priority(packet);
  }

  int packet_count{0};
  while (!sink.empty()) {
    int fragment_count{0};
    size_t payload_size_from_fragments{0};
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      const FragmentT& fragment = sink.fragment();
      payload_size_from_fragments += fragment.payload_size();
      ++fragment_count;
    }
    EXPECT_EQ(PAYLOAD_SIZE, payload_size_from_fragments);

    sink.pop_packet();
    ++packet_count;

    EXPECT_EQ(EXPECTED_FRAGMENT_COUNT, fragment_count);
  }

  EXPECT_EQ(4 * NUM_PACKETS_PER_QUEUE, packet_count);
}

TEST(FragmentSinkTest, AtExtremes) {
  static constexpr size_t MTU{8};
  static constexpr size_t NUM_PACKETS_PER_QUEUE{1024};
  static constexpr size_t MAX_FRAGMENTS_PER_PACKET{127};

  using FragmentT = tvsc::comms::radio::Fragment<MTU>;

  PacketTxQueue<JumboPacketT, NUM_PACKETS_PER_QUEUE> queue{};
  FragmentSink<JumboPacketT, NUM_PACKETS_PER_QUEUE, FragmentT::mtu(), MAX_FRAGMENTS_PER_PACKET>
      sink{queue.create_fragment_sink<FragmentT::mtu(), MAX_FRAGMENTS_PER_PACKET>()};

  static constexpr size_t EXPECTED_FRAGMENT_COUNT{MAX_FRAGMENTS_PER_PACKET};
  static constexpr size_t PAYLOAD_SIZE{FragmentT::max_payload_size() * EXPECTED_FRAGMENT_COUNT};

  // Assert that we have the correct values set for this test.
  static_assert(FragmentT::header_size() == 6);
  static_assert(FragmentT::payload_size_bytes_required() == 1);
  static_assert(FragmentT::max_payload_size() == 1);
  static_assert(JumboPacketT::max_payload_size() >= PAYLOAD_SIZE);
  static_assert(EXPECTED_FRAGMENT_COUNT <= MAX_FRAGMENTS_PER_PACKET);

  static_assert(JumboPacketT::header_size() + JumboPacketT::payload_size_bytes_required() +
                    PAYLOAD_SIZE >
                FragmentT::mtu());

  for (size_t i = 0; i < NUM_PACKETS_PER_QUEUE; ++i) {
    JumboPacketT packet{};
    packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_TDMA_CONTROL);
    packet.set_sender_id(static_cast<uint8_t>(i));

    packet.set_payload_length(PAYLOAD_SIZE);

    queue.push_immediate_priority(packet);
    queue.push_control_priority(packet);
    queue.push_normal_priority(packet);
    queue.push_low_priority(packet);
  }

  int packet_count{0};
  while (!sink.empty()) {
    int fragment_count{0};
    size_t payload_size_from_fragments{0};
    for (bool can_encode = sink.encode_next_packet(); sink.has_more_fragments();
         sink.next_fragment()) {
      ASSERT_TRUE(can_encode);
      const FragmentT& fragment = sink.fragment();
      payload_size_from_fragments += fragment.payload_size();
      ++fragment_count;
    }
    EXPECT_EQ(PAYLOAD_SIZE, payload_size_from_fragments);

    sink.pop_packet();
    ++packet_count;

    EXPECT_EQ(EXPECTED_FRAGMENT_COUNT, fragment_count);
  }

  EXPECT_EQ(4 * NUM_PACKETS_PER_QUEUE, packet_count);
}

}  // namespace tvsc::comms::packet
