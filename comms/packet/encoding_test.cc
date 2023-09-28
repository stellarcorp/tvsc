#include "comms/packet/encoding.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "base/except.h"
#include "comms/packet/packet.h"
#include "comms/packet/packet_assembler.h"
#include "comms/radio/fragment.h"
#include "comms/radio/protocol.h"
#include "gmock/gmock.h"

namespace tvsc::comms::packet {

std::vector<size_t> create_shuffle_vector(size_t num_fragments) {
  std::vector<size_t> result(num_fragments);
  LOG(INFO) << "num_fragments: " << num_fragments << ", result.size(): " << result.size();
  std::iota(result.begin(), result.end(), 0);
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(result.begin(), result.end(), g);

  return result;
}

template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
Packet roundtrip(const Packet& in) {
  using std::to_string;
  EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
  if (!encode(in, fragments)) {
    except<std::runtime_error>("Code not encode packet into fragments");
  }

  LOG(INFO) << "encoded packet: " << to_string(fragments) << "\n";

  // We assemble the fragments in a shuffled order to simulate fragments arriving out of order.
  // The shuffle_vector is a vector of indexes into the fragments structure above.
  std::vector<size_t> shuffle_vector{create_shuffle_vector(fragments.num_fragments)};

  // LOG the shuffle_vector.
  std::string shuffle_string{};
  shuffle_string.append("<");
  bool needs_comma{false};
  for (auto index : shuffle_vector) {
    if (needs_comma) {
      shuffle_string.append(", ");
    }
    shuffle_string.append(to_string(index));
    needs_comma = true;
  }
  shuffle_string.append(">");
  LOG(INFO) << "shuffle_vector: " << shuffle_string;

  PacketAssembler<Packet> assembler{};

  for (auto index : shuffle_vector) {
    LOG(INFO) << "decoding fragment index: " << index
              << " (Note: this is simulating receiving fragments out of order)";

    assembler.add_fragment(fragments.buffers[index]);
  }

  return assembler.consume_packet();
}

constexpr size_t LARGE_PACKET_MAX_PAYLOAD_SIZE{65000};

template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> roundtrip_large_packet(
    const PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE>& in) {
  using std::to_string;
  EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET> fragments{};
  encode(in, fragments);

  LOG(INFO) << "encoded packet: " << to_string(fragments) << "\n";

  // We assemble the fragments in a shuffled order to simulate fragments arriving out of order.
  // The shuffle_vector is a vector of indexes into the fragments structure above.
  std::vector<size_t> shuffle_vector{create_shuffle_vector(fragments.num_fragments)};

  std::vector<PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE>> packets{};
  for (auto index : shuffle_vector) {
    PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> p{};
    decode(fragments.buffers[index], p);
    packets.push_back(p);
  }

  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> out{};
  assemble(packets.begin(), packets.end(), out);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_normal_sizes(packet));
}

TEST(EncodingTest, CanEncodeNormalPacketWithoutPayload) {
  Packet packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_large_sizes(packet));
}

TEST(EncodingTest, CanEncodeLargePacketWithoutPayload) {
  PacketT<LARGE_PACKET_MAX_PAYLOAD_SIZE> packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_small_sizes(packet));
}

TEST(EncodingTest, CanEncodeSmallPacketWithoutPayload) {
  Packet packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  EXPECT_TRUE(roundtrip_gives_same_packet_tiny_sizes(packet));
}

TEST(EncodingTest, CanEncodeTinyPacketWithoutPayload) {
  Packet packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
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

TEST(EncodeTest, CanDetectPacketTooLargeForEncodingParameters) {
  Packet packet{};
  packet.set_protocol(tvsc::comms::radio::Protocol::TVSC_CONTROL);
  packet.set_sender_id(42);
  packet.set_destination_id(101);
  packet.set_sequence_number(15000);
  static constexpr size_t PAYLOAD_LENGTH{500};
  for (size_t i = 0; i < PAYLOAD_LENGTH; ++i) {
    packet.payload()[i] = static_cast<uint8_t>(i & 0xff);
  }
  packet.set_payload_length(PAYLOAD_LENGTH);

  // Note that the fragments are not large enough to hold the entire packet.
  EncodedPacket<50 /* MTU for fragments */, 5 /* Max fragments per packet*/> fragments{};
  bool success = encode(packet, fragments);

  EXPECT_FALSE(success);
}

TEST(DecodeTest, CanDetectErrantPacketLength) {
  constexpr size_t MTU{50};
  tvsc::comms::radio::Fragment<MTU> fragment{};
  fragment.set_payload_size(tvsc::comms::radio::Fragment<MTU>::max_payload_size() + 1);

  Packet packet{};
  bool success = decode(fragment, packet);

  EXPECT_FALSE(success);
}

}  // namespace tvsc::comms::packet
