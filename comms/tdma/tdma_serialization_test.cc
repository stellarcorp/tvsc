#include "comms/tdma/tdma_serialization.h"

#include "comms/tdma/tdma_frame.h"
#include "gtest/gtest.h"

namespace tvsc::comms::tdma {

TEST(FrameSerializationTest, CanRoundTripEmptyFrame) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  Frame in{};
  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

TEST(FrameSerializationTest, CanRoundTripFrameWithoutTimeSlots) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  Frame in{};
  in.frame_start_time_us = 7;
  in.base_station_id = 0xabcddcba;

  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

TEST(FrameSerializationTest, CanRoundTripFrameWithOneTimeSlot) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  FrameBuilder builder{7};
  builder.set_base_station_id(0xabcddcba);
  builder.add_guard_interval(1000);

  const Frame in{builder.build()};

  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

TEST(FrameSerializationTest, CanRoundTripFrameWithTwoTimeSlots) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  FrameBuilder builder{7};
  builder.set_base_station_id(0xabcd);
  builder.add_node_tx_slot(1000, 0xfed);
  builder.add_guard_interval(1000);

  const Frame in{builder.build()};

  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

TEST(FrameSerializationTest, CanRoundTripFrameWithManyTimeSlots) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  FrameBuilder builder{7};
  builder.set_base_station_id(0xabcd);
  builder.add_node_tx_slot(1000, 0xabcd);
  builder.add_guard_interval(1000);
  builder.add_node_tx_slot(1000, 0xfed);
  builder.add_guard_interval(1000);
  builder.add_association_slot(1000);
  builder.add_guard_interval(1000);
  builder.add_blackout_slot(1000);

  const Frame in{builder.build()};

  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

TEST(FrameSerializationTest, CanRoundTripDefaultNodeFrame) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const Frame in{FrameBuilder::create_default_node_frame()};

  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

TEST(FrameSerializationTest, CanRoundTripDefaultBaseStationFrame) {
  constexpr size_t BUFFER_SIZE{256};
  uint8_t buffer[BUFFER_SIZE];
  size_t encoded_size{BUFFER_SIZE};

  const Frame in{FrameBuilder::create_default_base_station_frame(0xabcd)};

  constexpr uint64_t IN_TIME_US{1};
  ASSERT_TRUE(encode(in, IN_TIME_US, buffer, encoded_size));
  ASSERT_LE(encoded_size, BUFFER_SIZE);

  Frame out{};
  uint64_t out_time_us{};

  ASSERT_TRUE(decode(out, out_time_us, buffer, encoded_size));

  EXPECT_EQ(in, out);
  EXPECT_EQ(IN_TIME_US, out_time_us);
}

}  // namespace tvsc::comms::tdma
