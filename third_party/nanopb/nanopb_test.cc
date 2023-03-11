#include <cstring>
#include <type_traits>
#include <vector>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "third_party/nanopb/sample_messages.pb.h"

template <typename MessageT, size_t BUFFER_SIZE = 128>
MessageT round_trip(const pb_msgdesc_t &message_descriptor, const MessageT &incoming) {
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

TEST(UnexpectedBehavior, AllocatesExtraByteForEachFixedLengthStringField) {
  // We request the field SIZE to be 128 bytes in the proto definition and get what we expect.
  EXPECT_EQ(128, sizeof(FixedStringLength::fixed_size));
  // But if we request the field LENGTH to be 127 bytes in the proto definition, but nanopb
  // allocates an extra byte for the terminating zero.
  EXPECT_EQ(127 + 1, sizeof(FixedStringLength::fixed_length));
}

TEST(ExplorationTest, CanAllocateStringsAsFixedSizeBuffers) {
  constexpr char GREETING[] = "Not another 'Hello, world!' message.";

  FixedStringLength message{};
  std::strncpy(message.fixed_size, GREETING, sizeof(message.fixed_size));

  FixedStringLength decoded = round_trip(FixedStringLength_msg, message);
  EXPECT_STREQ(GREETING, decoded.fixed_size);
}

TEST(ExplorationTest, MessageCopyCopiesTheValueNotTheAddress) {
  constexpr char GREETING1[] = "Hello, world!";
  constexpr char GREETING2[] = "Not another 'Hello, world!' message.";
  constexpr char GREETING3[] = "Is this field unique?";

  FixedStringLength message1{};
  FixedStringLength message2{};
  std::strncpy(message1.fixed_size, GREETING1, sizeof(message1.fixed_size));
  std::strncpy(message2.fixed_size, GREETING2, sizeof(message2.fixed_size));

  message1 = message2;

  // Overwrite message2.fixed_size to verify that message1 copies the value of the field in the
  // assignment, not the address.
  std::strncpy(message2.fixed_size, GREETING3, sizeof(message2.fixed_size));

  // If the fixed_size fields in both messages have the same address, we will have difficulties
  // working with this library.
  EXPECT_NE(message2.fixed_size, message1.fixed_size);

  // Check that the values in the fixed_size fields are what we expect.
  EXPECT_STREQ(GREETING2, message1.fixed_size);
  EXPECT_STREQ(GREETING3, message2.fixed_size);
}

bool RepeatedFieldMessage_callback(pb_istream_t *istream, pb_ostream_t *ostream,
                                   const pb_field_iter_t *field) {
  if (ostream != nullptr) {
    if (field->tag == RepeatedFieldMessage_bar_tag) {
      const auto &bar_values{*(const std::vector<int> *)field->pData};
      for (const auto &value : bar_values) {
        if (!pb_encode_tag_for_field(ostream, field)) {
          LOG(WARNING) << "Could not encode field tag";
          return false;
        }
        if (!pb_encode_varint(ostream, value)) {
          LOG(WARNING) << "Could not encode field value";
          return false;
        }
      }
    }
  } else if (istream != nullptr) {
    if (field->tag == RepeatedFieldMessage_bar_tag) {
      uint32_t value{};
      if (!pb_decode_varint32(istream, &value)) {
        LOG(WARNING) << "Could not decode field";
        return false;
      }
      auto &bar_values{*(std::vector<int> *)field->pData};
      bar_values.push_back(static_cast<int>(value));
    }
  }

  return true;
}

TEST(ExplorationTest, DemonstrationOfRepeatedFieldCallback) {
  RepeatedFieldMessage msg1{};
  for (int i = 0; i < 10; ++i) {
    msg1.bar.push_back(i);
  }

  RepeatedFieldMessage msg2 = round_trip(RepeatedFieldMessage_msg, msg1);
  EXPECT_EQ(msg1.bar, msg2.bar);
}

template <typename MessageT>
::testing::AssertionResult has_expected_traits() {
  if (!std::is_trivially_constructible<TrivialMessage>::value) {
    return ::testing::AssertionFailure();
  }
  if (!std::is_trivially_default_constructible<TrivialMessage>::value) {
    return ::testing::AssertionFailure();
  }
  if (!std::is_trivially_copyable<TrivialMessage>::value) {
    return ::testing::AssertionFailure();
  }

  constexpr bool is_trivially_assignable{
      std::is_trivially_assignable<TrivialMessage, TrivialMessage>::value};
  if (!is_trivially_assignable) {
    return ::testing::AssertionFailure();
  }

  if (!std::is_trivially_copy_assignable<TrivialMessage>::value) {
    return ::testing::AssertionFailure();
  }
  if (!std::is_trivially_move_assignable<TrivialMessage>::value) {
    return ::testing::AssertionFailure();
  }

  return ::testing::AssertionSuccess();
}

TEST(ExplorationTest, NanoPbMessagesHaveExpectedTypeTraits) {
  EXPECT_TRUE(has_expected_traits<TrivialMessage>());
  EXPECT_TRUE(has_expected_traits<Complicated>());
  EXPECT_TRUE(has_expected_traits<FixedStringLength>());
  EXPECT_TRUE(has_expected_traits<RepeatedFieldMessage>());
  EXPECT_TRUE(has_expected_traits<OneOfMessage>());
}
