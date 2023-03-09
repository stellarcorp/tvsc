#include <cstring>
#include <type_traits>

#include "gtest/gtest.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "third_party/nanopb/sample_messages.pb.h"

template <typename MessageT, size_t BUFFER_SIZE = 128>
MessageT round_trip(const pb_msgdesc_t& message_descriptor, const MessageT& incoming) {
  uint8_t buffer[BUFFER_SIZE];
  size_t message_length{0};
  bool status{false};

  pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  status = pb_encode(&ostream, &message_descriptor, &incoming);
  if (!status) {
    throw std::runtime_error("Could not encode incoming message");
  }
  message_length = ostream.bytes_written;

  pb_istream_t istream = pb_istream_from_buffer(buffer, message_length);

  MessageT outgoing{};
  status = pb_decode(&istream, &message_descriptor, &outgoing);
  if (!status) {
    throw std::runtime_error("Could not decode to outgoing message");
  }

  return outgoing;
}

TEST(CompileTest, CanRoundTripTrivialMessage) {
  constexpr int LUCKY_NUMBER{14};

  TrivialMessage message{};
  message.lucky_number = LUCKY_NUMBER;
  TrivialMessage decoded = round_trip(TrivialMessage_msg, message);

  EXPECT_EQ(LUCKY_NUMBER, decoded.lucky_number);
}

TEST(UnexpectedBehavior, AllocatesExtraByteForEachFixedSizeField) {
  // We request the field size to be 127 bytes in the proto definition.
  EXPECT_EQ(127 + 1, sizeof(FixedStringLength::foo));
}

TEST(ExplorationTest, CanAllocateStringsAsFixedSizeBuffers) {
  constexpr char GREETING[] = "Not another 'Hello, world!' message.";

  FixedStringLength message{};
  std::strncpy(message.foo, GREETING, sizeof(message.foo));

  FixedStringLength decoded = round_trip(FixedStringLength_msg, message);
  EXPECT_STREQ(GREETING, decoded.foo);
}

TEST(ExplorationTest, MessageCopyCopiesTheValueNotTheAddress) {
  constexpr char GREETING1[] = "Hello, world!";
  constexpr char GREETING2[] = "Not another 'Hello, world!' message.";
  constexpr char GREETING3[] = "Is this field unique?";

  FixedStringLength message1{};
  FixedStringLength message2{};
  std::strncpy(message1.foo, GREETING1, sizeof(message1.foo));
  std::strncpy(message2.foo, GREETING2, sizeof(message2.foo));

  message1 = message2;

  // Overwrite message2.foo to verify that message1 copies the value of the field in the assignment,
  // not the address.
  std::strncpy(message2.foo, GREETING3, sizeof(message2.foo));

  // If the foo fields in both messages have the same address, we will have difficulties working
  // with this library.
  EXPECT_NE(message2.foo, message1.foo);

  // Check that the values in the foo fields are what we expect.
  EXPECT_STREQ(GREETING2, message1.foo);
  EXPECT_STREQ(GREETING3, message2.foo);
}

TEST(ExplorationTest, NanoPbMessagesAreTriviallyConstructible) {
  EXPECT_TRUE(std::is_trivially_constructible<TrivialMessage>::value);
  EXPECT_TRUE(std::is_trivially_constructible<FixedStringLength>::value);
}

TEST(ExplorationTest, NanoPbMessagesAreTriviallyDefaultConstructible) {
  EXPECT_TRUE(std::is_trivially_default_constructible<TrivialMessage>::value);
  EXPECT_TRUE(std::is_trivially_default_constructible<FixedStringLength>::value);
}

TEST(ExplorationTest, NanoPbMessagesAreTriviallyCopyable) {
  EXPECT_TRUE(std::is_trivially_copyable<TrivialMessage>::value);
  EXPECT_TRUE(std::is_trivially_copyable<FixedStringLength>::value);
}

TEST(ExplorationTest, NanoPbMessagesAreTriviallyAssignable) {
  constexpr bool msg1{std::is_trivially_assignable<TrivialMessage, TrivialMessage>::value};
  constexpr bool msg2{std::is_trivially_assignable<FixedStringLength, FixedStringLength>::value};
  EXPECT_TRUE(msg1);
  EXPECT_TRUE(msg2);
}

TEST(ExplorationTest, NanoPbMessagesAreTriviallyCopyAssignable) {
  EXPECT_TRUE(std::is_trivially_copy_assignable<TrivialMessage>::value);
  EXPECT_TRUE(std::is_trivially_copy_assignable<FixedStringLength>::value);
}

TEST(ExplorationTest, NanoPbMessagesAreTriviallyMoveAssignable) {
  EXPECT_TRUE(std::is_trivially_move_assignable<TrivialMessage>::value);
  EXPECT_TRUE(std::is_trivially_move_assignable<FixedStringLength>::value);
}
