#include "radio/packet.h"

#include <cstring>

#include "gtest/gtest.h"

namespace tvsc::radio {

TEST(PacketTest, PayloadInitiallyEmpty) {
  Packet packet{};
  EXPECT_EQ(0, packet.payload()[0]);
}

}  // namespace tvsc::radio
