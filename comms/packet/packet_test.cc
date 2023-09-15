#include "comms/packet/packet.h"

#include <cstring>

#include "comms/radio/protocol.h"
#include "gtest/gtest.h"

namespace tvsc::comms::packet {

TEST(PacketTest, DefaultProtocolIsInet) {
  Packet packet{};
  EXPECT_EQ(tvsc::comms::radio::Protocol::INET, packet.protocol());
}

TEST(PacketTest, PayloadInitiallyEmpty) {
  Packet packet{};
  EXPECT_EQ(0, packet.payload()[0]);
}

TEST(PacketTest, PacketIsTriviallyCopyable) { EXPECT_TRUE(std::is_trivially_copyable<Packet>()); }

}  // namespace tvsc::comms::packet
