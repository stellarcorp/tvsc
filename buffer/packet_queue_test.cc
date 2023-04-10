#include "buffer/packet_queue.h"

#include <array>

#include "gmock/gmock.h"

namespace tvsc::buffer {

using TypicalPacketT = std::array<unsigned char, 64>;

TEST(PacketQueueTest, CanCompile) {
  PacketQueue<unsigned long, 128> queue{};

  EXPECT_TRUE(queue.empty());
}

TEST(PacketQueueTest, CanUseContainersAsPacketTypes) {
  PacketQueue<TypicalPacketT, 128> queue{};

  EXPECT_TRUE(queue.empty());
}

TEST(PacketQueueTest, CanAddPacketFundamentalType) {
  PacketQueue<unsigned char, 128> queue{};

  queue.push_normal('a');

  EXPECT_FALSE(queue.empty());
}

TEST(PacketQueueTest, CanAddPacket) {
  PacketQueue<TypicalPacketT, 128> queue{};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_normal(packet);

  EXPECT_FALSE(queue.empty());
}

TEST(PacketQueueTest, ReturnsSingleImmediatePacket) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_immediate(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketQueueTest, ReturnsSingleControlPacket) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_control(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketQueueTest, ReturnsSingleNormalPacket) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_normal(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketQueueTest, ReturnsSingleLowPacket) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_low_priority(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketQueueTest, ReturnsImmediatePacketEvenIfOtherPacketsAvailable) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT immediate_packet{};
  immediate_packet[0] = 42;

  TypicalPacketT control_packet{};
  control_packet[0] = 43;

  TypicalPacketT normal_packet{};
  normal_packet[0] = 44;

  TypicalPacketT low_packet{};
  low_packet[0] = 45;

  queue.push_immediate(immediate_packet);
  queue.push_control(control_packet);
  queue.push_normal(normal_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(immediate_packet, sink.peek());
}

TEST(PacketQueueTest, ReturnsPacketWhenImmediateNotAvailable) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT control_packet{};
  control_packet[0] = 43;

  TypicalPacketT normal_packet{};
  normal_packet[0] = 44;

  TypicalPacketT low_packet{};
  low_packet[0] = 45;

  queue.push_control(control_packet);
  queue.push_normal(normal_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == control_packet || packet == normal_packet || packet == low_packet);
}

TEST(PacketQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingControl) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT normal_packet{};
  normal_packet[0] = 44;

  TypicalPacketT low_packet{};
  low_packet[0] = 45;

  queue.push_normal(normal_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == normal_packet || packet == low_packet);
}

TEST(PacketQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingNormal) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT control_packet{};
  control_packet[0] = 43;

  TypicalPacketT low_packet{};
  low_packet[0] = 45;

  queue.push_control(control_packet);
  queue.push_low_priority(low_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == control_packet || packet == low_packet);
}

TEST(PacketQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingLow) {
  PacketQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT control_packet{};
  control_packet[0] = 43;

  TypicalPacketT normal_packet{};
  normal_packet[0] = 44;

  queue.push_control(control_packet);
  queue.push_normal(normal_packet);

  ASSERT_FALSE(sink.empty());
  const TypicalPacketT& packet = sink.peek();
  EXPECT_TRUE(packet == control_packet || packet == normal_packet);
}

}  // namespace tvsc::buffer
