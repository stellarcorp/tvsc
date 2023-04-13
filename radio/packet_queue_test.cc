#include "radio/packet_queue.h"

#include <array>

#include "gmock/gmock.h"

namespace tvsc::radio {

using TypicalPacketT = std::array<unsigned char, 64>;

TEST(PacketTxQueueTest, CanCompile) {
  PacketTxQueue<unsigned long, 128> queue{};

  EXPECT_TRUE(queue.empty());
}

TEST(PacketTxQueueTest, CanUseContainersAsPacketTypes) {
  PacketTxQueue<TypicalPacketT, 128> queue{};

  EXPECT_TRUE(queue.empty());
}

TEST(PacketTxQueueTest, CanAddPacketFundamentalType) {
  PacketTxQueue<unsigned char, 128> queue{};

  queue.push_normal('a');

  EXPECT_FALSE(queue.empty());
}

TEST(PacketTxQueueTest, CanAddPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_normal(packet);

  EXPECT_FALSE(queue.empty());
}

TEST(PacketTxQueueTest, ReturnsSingleImmediatePacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_immediate(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsSingleControlPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_control(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsSingleNormalPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_normal(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsSingleLowPacket) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
  PacketSink<TypicalPacketT, 128> sink{queue.create_sink()};

  TypicalPacketT packet{};
  packet[0] = 42;

  queue.push_low_priority(packet);

  ASSERT_FALSE(sink.empty());
  EXPECT_EQ(packet, sink.peek());
}

TEST(PacketTxQueueTest, ReturnsImmediatePacketEvenIfOtherPacketsAvailable) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
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

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailable) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
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

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingControl) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
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

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingNormal) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
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

TEST(PacketTxQueueTest, ReturnsPacketWhenImmediateNotAvailableMissingLow) {
  PacketTxQueue<TypicalPacketT, 128> queue{};
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

}  // namespace tvsc::radio
