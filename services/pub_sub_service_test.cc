#include "services/pub_sub_service.h"

#include <memory>
#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "services/test_mock.grpc.pb.h"
#include "services/test_response_reader.h"

namespace tvsc::services {

using ::testing::_;
using ::testing::ContainerEq;
using ::testing::Return;

constexpr char TOPIC_NAME[] = "topic";

template <typename MessageT>
class TestTopic final : public Topic<MessageT> {
 private:
  MessageT last_message_{};
  std::function<void(const MessageT& message)> callback_{};

  void publish_compressed(std::string_view msg) override {
    last_message_.ParseFromString(std::string{msg});
    if (callback_) {
      callback_(last_message_);
    }
  }
  void publish_uncompressed(std::string_view msg) override {
    last_message_.ParseFromString(std::string{msg});
    if (callback_) {
      callback_(last_message_);
    }
  }

 public:
  TestTopic() : Topic<MessageT>(TOPIC_NAME){};
  TestTopic(std::string_view topic_name) : Topic<MessageT>(topic_name){};

  const MessageT& last_message() const { return last_message_; }

  void register_callback(std::function<void(const MessageT& message)> callback) {
    callback_ = std::move(callback);
  }
};

void add_test_message(TestReader<TestResponse>* backend, std::string message) {
  TestResponse response{};
  response.set_message(std::move(message));
  backend->add_response(response);
}

TEST(PubSubServiceTest, CanPublishRpcResponseOverTopic) {
  constexpr char MESSAGE[]{"message"};

  MockTestServiceStub stub;

  {
    std::unique_ptr<TestReader<TestResponse>> backend{std::make_unique<TestReader<TestResponse>>()};
    add_test_message(backend.get(), MESSAGE);

    EXPECT_CALL(stub, server_streamRaw(_, _)).WillOnce(Return(backend.release()));
  }

  TestTopic<TestResponse> topic{};
  PubSubService<TestRequest, TestResponse> service{
      topic, [&stub](grpc::ClientContext* context, const TestRequest& request) {
        return stub.server_stream(context, request);
      }};

  service.loop();

  EXPECT_EQ(MESSAGE, topic.last_message().message());
}

TEST(PubSubServiceTest, CanPublishRpcStreamOverTopic) {
  std::vector<TestResponse> expected{};
  for (int i = 0; i < 10; ++i) {
    using std::to_string;
    TestResponse message{};
    message.set_message("Message " + to_string(i));
    expected.emplace_back(message);
  }

  MockTestServiceStub stub;

  {
    std::unique_ptr<TestReader<TestResponse>> backend{std::make_unique<TestReader<TestResponse>>()};
    for (const auto& msg : expected) {
      backend->add_response(msg);
    }

    EXPECT_CALL(stub, server_streamRaw(_, _)).WillOnce(Return(backend.release()));
  }

  TestTopic<TestResponse> topic{};

  std::vector<TestResponse> responses{};
  topic.register_callback(
      [&responses](const TestResponse& message) { responses.emplace_back(message); });

  PubSubService<TestRequest, TestResponse> service{
      topic, [&stub](grpc::ClientContext* context, const TestRequest& request) {
        return stub.server_stream(context, request);
      }};

  service.loop();

  EXPECT_EQ(expected.size(), responses.size());
  for (uint32_t i = 0; i < responses.size(); ++i) {
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equivalent(expected[i], responses[i]));
  }
}

}  // namespace tvsc::services
