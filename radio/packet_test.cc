#include "radio/packet.h"

#include <cstring>

#include "gtest/gtest.h"

namespace tvsc::radio {

TEST(PacketTest, DefaultProtocolIsInet) {
  Packet packet{};
  EXPECT_EQ(Protocol::INET, packet.protocol());
}

TEST(PacketTest, PayloadInitiallyEmpty) {
  Packet packet{};
  EXPECT_EQ(0, packet.payload()[0]);
}

TEST(PacketTest, PacketIsTriviallyCopyable) { EXPECT_TRUE(std::is_trivially_copyable<Packet>()); }

}  // namespace tvsc::radio
