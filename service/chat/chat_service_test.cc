#include <chrono>
#include <memory>

#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "service/chat/client/client.h"
#include "service/chat/common/chat.grpc.pb.h"
#include "service/chat/common/chat.pb.h"
#include "service/chat/server/server.h"
#include "service/utility/service_runner.h"
#include "service/utility/status_testing.h"

namespace tvsc::service::chat {

using namespace std::literals::chrono_literals;
using Clock = std::chrono::system_clock;

class ChatServiceTest : public ::testing::Test {
 public:
  tvsc::service::utility::ServiceRunner service_runner{};
  ChatServiceImpl service{};

  ChatServiceTest() { service_runner.add_service<Chat, ChatServiceImpl>("Chat Service", service); }

  void SetUp() override { service_runner.start(); }

  void TearDown() override { service_runner.stop(); }
};

TEST_F(ChatServiceTest, CanCallPost) {
  constexpr char POSTER_NAME[] = "poster";
  constexpr char MESSAGE[] = "Hello, world!";

  ChatMessage message{};
  message.set_poster(POSTER_NAME);
  message.set_message(MESSAGE);

  ChatClient client{service_runner.bind_address()};

  EXPECT_OK(client.post(message));
}

TEST_F(ChatServiceTest, CanSubscribeAndGetSingleMessage) {
  constexpr char POSTER_NAME[] = "poster";
  constexpr char MESSAGE[] = "Hello, world!";

  ChatMessage outgoing_message{};
  outgoing_message.set_poster(POSTER_NAME);
  outgoing_message.set_message(MESSAGE);

  ChatClient subscribed_client{service_runner.bind_address()};
  ChatClient poster{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // Get all of the messages we can in 100ms.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<ChatMessage>> reader{};
  reader = subscribed_client.subscribe_to_messages(subscribed_context);

  EXPECT_OK(poster.post(outgoing_message));

  ChatMessage received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_EQ(POSTER_NAME, received_message.poster());
    EXPECT_EQ(MESSAGE, received_message.message());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(ChatServiceTest, CanSubscribeAndGetManyMessages) {
  constexpr char POSTER_PREFIX[] = "poster";
  constexpr char MESSAGE_PREFIX[] = "Hello, world!";
  constexpr int NUM_MESSAGES{20};

  ChatClient subscribed_client{service_runner.bind_address()};
  ChatClient poster{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // Get all of the messages we can before timing out.
  subscribed_context.set_deadline(Clock::now() + 250ms);

  std::unique_ptr<grpc::ClientReaderInterface<ChatMessage>> reader{};
  reader = subscribed_client.subscribe_to_messages(subscribed_context);

  std::vector<ChatMessage> sent_messages{};
  for (int i = 0; i < NUM_MESSAGES; ++i) {
    using std::to_string;

    ChatMessage outgoing_message{};
    outgoing_message.set_poster(POSTER_PREFIX + to_string(i));
    outgoing_message.set_message(MESSAGE_PREFIX + to_string(i));

    ASSERT_OK(poster.post(outgoing_message));
    sent_messages.push_back(outgoing_message);
  }

  ChatMessage received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;

    const ChatMessage& sent_message{sent_messages.front()};
    EXPECT_EQ(sent_message.poster(), received_message.poster());
    EXPECT_EQ(sent_message.message(), received_message.message());
    sent_messages.erase(sent_messages.begin());

    if (read_message_count == NUM_MESSAGES) {
      subscribed_context.TryCancel();
    }
  }

  EXPECT_EQ(NUM_MESSAGES, read_message_count);
}

TEST_F(ChatServiceTest, PosterReceivesOwnMessageSingle) {
  constexpr char POSTER_NAME[] = "poster";
  constexpr char MESSAGE[] = "Hello, world!";

  ChatMessage outgoing_message{};
  outgoing_message.set_poster(POSTER_NAME);
  outgoing_message.set_message(MESSAGE);

  ChatClient poster{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // Get all of the messages we can in 100ms.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<ChatMessage>> reader{};
  reader = poster.subscribe_to_messages(subscribed_context);

  EXPECT_OK(poster.post(outgoing_message));

  ChatMessage received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_EQ(POSTER_NAME, received_message.poster());
    EXPECT_EQ(MESSAGE, received_message.message());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(ChatServiceTest, PosterReceivesOwnMessagesMany) {
  constexpr char POSTER_PREFIX[] = "poster";
  constexpr char MESSAGE_PREFIX[] = "Hello, world!";
  constexpr int NUM_MESSAGES{20};

  ChatClient poster{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // Get all of the messages we can before timing out.
  subscribed_context.set_deadline(Clock::now() + 250ms);

  std::unique_ptr<grpc::ClientReaderInterface<ChatMessage>> reader{};
  reader = poster.subscribe_to_messages(subscribed_context);

  std::vector<ChatMessage> sent_messages{};
  for (int i = 0; i < NUM_MESSAGES; ++i) {
    using std::to_string;

    ChatMessage outgoing_message{};
    outgoing_message.set_poster(POSTER_PREFIX + to_string(i));
    outgoing_message.set_message(MESSAGE_PREFIX + to_string(i));

    ASSERT_OK(poster.post(outgoing_message));
    sent_messages.push_back(outgoing_message);
  }

  ChatMessage received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;

    const ChatMessage& sent_message{sent_messages.front()};
    EXPECT_EQ(sent_message.poster(), received_message.poster());
    EXPECT_EQ(sent_message.message(), received_message.message());
    sent_messages.erase(sent_messages.begin());

    if (read_message_count == NUM_MESSAGES) {
      subscribed_context.TryCancel();
    }
  }

  EXPECT_EQ(NUM_MESSAGES, read_message_count);
}

}  // namespace tvsc::service::chat
