#include "radio/encoding.h"

#include "gmock/gmock.h"
#include "radio/packet.h"

namespace tvsc::radio {

template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
Packet roundtrip(const Packet& in) {
  using std::to_string;
  EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
  encode(in, fragments);

  LOG(INFO) << "encoded packet: " << to_string(fragments) << "\n";

  Packet out{};

  assemble(fragments, out);
  return out;
}
constexpr size_t LARGE_PACKET_MAX_PAYLOAD_SIZE{65000};

template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> roundtrip_large_packet(
    const PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE>& in) {
  using std::to_string;
  EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
  encode(in, fragments);

  LOG(INFO) << "encoded packet: " << to_string(fragments) << "\n";

  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> out{};

  assemble(fragments, out);
  return out;
}

constexpr size_t TINY_MTU{32};
constexpr size_t TINY_FRAGMENT_MAXIMUM{4};
constexpr size_t SMALL_MTU{64};
constexpr size_t SMALL_FRAGMENT_MAXIMUM{4};
constexpr size_t NORMAL_MTU{255};
constexpr size_t NORMAL_FRAGMENT_MAXIMUM{16};
constexpr size_t LARGE_MTU{16383};
constexpr size_t LARGE_FRAGMENT_MAXIMUM{127};

::testing::AssertionResult roundtrip_gives_same_packet_tiny_sizes(const Packet& in) {
  LOG(INFO) << "packet: " << to_string(in);
  const Packet out{roundtrip<TINY_MTU, TINY_FRAGMENT_MAXIMUM>(in)};
  if (in == out) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "in: " << to_string(in) << ", out: " << to_string(out);
  }
}

::testing::AssertionResult roundtrip_gives_same_packet_small_sizes(const Packet& in) {
  LOG(INFO) << "packet: " << to_string(in);
  const Packet out{roundtrip<SMALL_MTU, SMALL_FRAGMENT_MAXIMUM>(in)};
  if (in == out) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "in: " << to_string(in) << ", out: " << to_string(out);
  }
}

::testing::AssertionResult roundtrip_gives_same_packet_normal_sizes(const Packet& in) {
  LOG(INFO) << "packet: " << to_string(in);
  const Packet out{roundtrip<NORMAL_MTU, NORMAL_FRAGMENT_MAXIMUM>(in)};
  if (in == out) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "in: " << to_string(in) << ", out: " << to_string(out);
  }
}

::testing::AssertionResult roundtrip_gives_same_packet_large_sizes(
    const PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE>& in) {
  LOG(INFO) << "packet: " << to_string(in);
  const PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> out{
      roundtrip_large_packet<LARGE_MTU, LARGE_FRAGMENT_MAXIMUM>(in)};
  if (in == out) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "in: " << to_string(in) << ", out: " << to_string(out);
  }
}

TEST(EncodingTest, CanEncodeTrivialNormalPacket) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_normal_sizes(packet));
}

TEST(EncodingTest, CanEncodeNormalPacketWithoutPayload) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  EXPECT_TRUE(roundtrip_gives_same_packet_normal_sizes(packet));
}

TEST(EncodingTest, CanEncodeNormalPacketWithoutPayloadTrivialHeaders) {
  Packet packet{};
  EXPECT_TRUE(roundtrip_gives_same_packet_normal_sizes(packet));
}

TEST(EncodingTest, CanEncodeNormalPacket) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{10};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_normal_sizes(packet));
}

TEST(EncodingTest, CanEncodeNormalPacketWithPayloadLargerThanMtu) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{2 * NORMAL_MTU + 11};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_normal_sizes(packet));
}

TEST(EncodingTest, CanEncodeTrivialLargePacket) {
  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_large_sizes(packet));
}

TEST(EncodingTest, CanEncodeLargePacketWithoutPayload) {
  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  EXPECT_TRUE(roundtrip_gives_same_packet_large_sizes(packet));
}

TEST(EncodingTest, CanEncodeLargePacketWithoutPayloadTrivialHeaders) {
  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> packet{};
  EXPECT_TRUE(roundtrip_gives_same_packet_large_sizes(packet));
}

TEST(EncodingTest, CanEncodeLargePacket) {
  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{10};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_large_sizes(packet));
}

TEST(EncodingTest, CanEncodeLargePacketWithPayloadLargerThanMtu) {
  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{2 * LARGE_MTU + 11};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_large_sizes(packet));
}

TEST(EncodingTest, CanEncodeTrivialSmallPacket) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_small_sizes(packet));
}

TEST(EncodingTest, CanEncodeSmallPacketWithoutPayload) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  EXPECT_TRUE(roundtrip_gives_same_packet_small_sizes(packet));
}

TEST(EncodingTest, CanEncodeSmallPacketWithoutPayloadTrivialHeaders) {
  Packet packet{};
  EXPECT_TRUE(roundtrip_gives_same_packet_small_sizes(packet));
}

TEST(EncodingTest, CanEncodeSmallPacket) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{10};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_small_sizes(packet));
}

TEST(EncodingTest, CanEncodeSmallPacketWithPayloadLargerThanMtu) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{2 * SMALL_MTU + 11};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_small_sizes(packet));
}

TEST(EncodingTest, CanEncodeTrivialTinyPacket) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_tiny_sizes(packet));
}

TEST(EncodingTest, CanEncodeTinyPacketWithoutPayload) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  EXPECT_TRUE(roundtrip_gives_same_packet_tiny_sizes(packet));
}

TEST(EncodingTest, CanEncodeTinyPacketWithoutPayloadTrivialHeaders) {
  Packet packet{};
  EXPECT_TRUE(roundtrip_gives_same_packet_tiny_sizes(packet));
}

TEST(EncodingTest, CanEncodeTinyPacket) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{10};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_tiny_sizes(packet));
}

TEST(EncodingTest, CanEncodeTinyPacketWithPayloadLargerThanMtu) {
  Packet packet{};
  packet.set_protocol(Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{2 * TINY_MTU + 11};
  packet.set_payload_length(PAYLOAD_LENGTH);
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  EXPECT_TRUE(roundtrip_gives_same_packet_tiny_sizes(packet));
}

}  // namespace tvsc::radio
