#include "gmock/gmock.h"
#include "service/chat/client/client.h"
#include "service/chat/common/chat.grpc.pb.h"
#include "service/chat/common/chat.pb.h"
#include "service/chat/server/server.h"
#include "service/utility/service_runner.h"
#include "service/utility/status_testing.h"
#include "discovery/service_resolver.h"

namespace tvsc::service::chat {

class ChatServiceTest : public ::testing::Test {
 public:
  tvsc::service::utility::ServiceRunner service_runner{};
  ChatServiceImpl service{};

  ChatServiceTest() {
    tvsc::discovery::register_mdns_grpc_resolver();
    service_runner.add_service<Chat, ChatServiceImpl>("Chat Service", service);
  }

  void SetUp() override { service_runner.start(); }

  void TearDown() override { service_runner.stop(); }
};

TEST_F(ChatServiceTest, CanCallPost) {
  constexpr char POSTER_NAME[] = "poster";
  constexpr char MESSAGE[] = "Hello, world!";

  ChatMessage message{};
  message.set_poster(POSTER_NAME);
  message.set_message(MESSAGE);

  ChatClient client{};

  EXPECT_OK(client.post(message));
}

}  // namespace tvsc::service::chat
