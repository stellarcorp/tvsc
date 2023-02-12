#include "pubsub/pub_sub_service.h"

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#include "glog/logging.h"
#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "grpcpp/create_channel.h"
#include "grpcpp/server_builder.h"
#include "pubsub/streamer.h"
#include "pubsub/test.grpc.pb.h"
#include "pubsub/topic.h"

namespace tvsc::pubsub {

using ::testing::_;
using ::testing::ContainerEq;
using ::testing::DoAll;
using ::testing::IgnoreResult;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::WithArgs;

using namespace std::literals::chrono_literals;

constexpr char TOPIC_NAME[] = "topic";

template <typename MessageT>
class TestTopic final : public Topic<MessageT> {
 private:
  MessageT last_message_{};
  std::function<void(const MessageT& message)> callback_{};

 public:
  TestTopic() : Topic<MessageT>(TOPIC_NAME){};
  TestTopic(std::string_view topic_name) : Topic<MessageT>(topic_name){};

  void publish(const MessageT& message) override {
    last_message_ = message;
    if (callback_) {
      callback_(last_message_);
    }
  }

  const MessageT& last_message() const { return last_message_; }

  void register_callback(std::function<void(const MessageT& message)> callback) {
    callback_ = std::move(callback);
  }
};

class TestServiceImpl : public TestService::Service {
 public:
  std::vector<TestResponse> canned_responses{};

  grpc::Status server_stream(grpc::ServerContext* /*context*/, const TestRequest* /*request*/,
                             grpc::ServerWriter<TestResponse>* writer) override {
    for (const auto& response : canned_responses) {
      LOG(INFO) << "Server::server_stream() -- writing response: " << response.DebugString();
      std::this_thread::sleep_for(10ms);
      writer->Write(response);
    }
    return grpc::Status::OK;
  }
};

class TestStreamer final : public GrpcStreamer<TestService, TestRequest, TestResponse> {
 protected:
  void call_rpc_method(TestService::StubInterface::async_interface& async_stub,
                       grpc::ClientContext& context, const TestRequest& request,
                       grpc::ClientReadReactor<TestResponse>& reactor) override {
    LOG(INFO) << "TestStreamer::call_rpc_method()";
    async_stub.server_stream(&context, &request, &reactor);
  }

 public:
  TestStreamer(const std::string& bind_addr) : GrpcStreamer(bind_addr) {}
};

class PublicationServiceTest : public ::testing::Test {
 public:
  std::ostringstream server_address{};
  std::unique_ptr<grpc::Server> server{};

  TestServiceImpl service{};

  void SetUp() override {
    LOG(INFO) << "PublicationServiceTest::SetUp()";
    grpc::ServerBuilder builder;
    int port{};
    builder.AddListeningPort("localhost:0", grpc::InsecureServerCredentials(), &port);
    builder.RegisterService(&service);
    server = builder.BuildAndStart();

    LOG(INFO) << "PublicationServiceTest::SetUp() -- Listening on port " << port;

    server_address << "localhost:" << port;
  }

  void TearDown() override { server->Shutdown(); }

  void add_canned_response(std::string_view text) {
    TestResponse response{};
    response.set_message(std::string{text});
    service.canned_responses.emplace_back(std::move(response));
  }
};

TEST_F(PublicationServiceTest, CanPublishRpcResponseOverTopic) {
  constexpr char MESSAGE[]{"message"};
  add_canned_response(MESSAGE);

  TestTopic<TestResponse> topic{};
  PublicationService<TestResponse> pub_sub{topic,
                                           std::make_unique<TestStreamer>(server_address.str())};

  pub_sub.start();
  pub_sub.await();

  EXPECT_TRUE(pub_sub.status().ok());
  EXPECT_EQ(MESSAGE, topic.last_message().message());
}

TEST_F(PublicationServiceTest, CanPublishRpcStreamOverTopic) {
  for (int i = 0; i < 3; ++i) {
    using std::to_string;
    add_canned_response("Message " + to_string(i));
  }

  TestTopic<TestResponse> topic{};

  std::vector<TestResponse> responses{};
  topic.register_callback(
      [&responses](const TestResponse& message) { responses.emplace_back(message); });

  PublicationService<TestResponse> pub_sub{topic,
                                           std::make_unique<TestStreamer>(server_address.str())};

  pub_sub.start();
  pub_sub.await();

  EXPECT_TRUE(pub_sub.status().ok());
  EXPECT_EQ(service.canned_responses.size(), responses.size());
  for (uint32_t i = 0; i < responses.size(); ++i) {
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equivalent(service.canned_responses[i],
                                                                       responses[i]));
  }
}

TEST_F(PublicationServiceTest, CanPublishLongRpcStreamOverTopic) {
  for (int i = 0; i < 20; ++i) {
    using std::to_string;
    add_canned_response("Message " + to_string(i));
  }

  TestTopic<TestResponse> topic{};

  std::vector<TestResponse> responses{};
  topic.register_callback(
      [&responses](const TestResponse& message) { responses.emplace_back(message); });

  PublicationService<TestResponse> pub_sub{topic,
                                           std::make_unique<TestStreamer>(server_address.str())};

  pub_sub.start();
  pub_sub.await();

  EXPECT_TRUE(pub_sub.status().ok());
  EXPECT_EQ(service.canned_responses.size(), responses.size());
  for (uint32_t i = 0; i < responses.size(); ++i) {
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equivalent(service.canned_responses[i],
                                                                       responses[i]));
  }
}

}  // namespace tvsc::pubsub
