#include "services/pub_sub_service.h"

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
#include "services/test.grpc.pb.h"

namespace tvsc::services {

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

class TestServiceImpl : public TestService::Service {
 public:
  std::vector<TestResponse> canned_responses{};

  grpc::Status server_stream(grpc::ServerContext* /*context*/, const TestRequest* /*request*/,
                             grpc::ServerWriter<TestResponse>* writer) override {
    for (const auto& response : canned_responses) {
      DLOG(INFO) << "Server::writing response: " << response.DebugString();
      std::this_thread::sleep_for(10ms);
      writer->Write(response);
    }
    return grpc::Status::OK;
  }
};

class PubSubServiceTest : public ::testing::Test {
 public:
  std::ostringstream server_address{};
  std::unique_ptr<grpc::Server> server{};

  TestServiceImpl service{};

  std::shared_ptr<grpc::Channel> channel{};
  std::unique_ptr<TestService::Stub> client_stub{};

  void SetUp() override {
    DLOG(INFO) << "PubSubServiceTest::SetUp()";
    grpc::ServerBuilder builder;
    int port{};
    builder.AddListeningPort("localhost:0", grpc::InsecureServerCredentials(), &port);
    builder.RegisterService(&service);
    server = builder.BuildAndStart();

    DLOG(INFO) << "PubSubServiceTest::SetUp() -- Listening on port " << port;

    server_address << "localhost:" << port;
    channel = grpc::CreateChannel(server_address.str(), grpc::InsecureChannelCredentials());
    client_stub = TestService::NewStub(channel);

    DLOG(INFO) << "PubSubServiceTest::SetUp() -- Client connecting to " << server_address.str();
  }

  void TearDown() override {
    client_stub.reset();
    channel.reset();
    server->Shutdown();
  }

  void add_canned_response(std::string_view text) {
    TestResponse response{};
    response.set_message(std::string{text});
    service.canned_responses.emplace_back(std::move(response));
  }
};

TEST_F(PubSubServiceTest, CanPublishRpcResponseOverTopic) {
  constexpr char MESSAGE[]{"message"};
  add_canned_response(MESSAGE);

  TestTopic<TestResponse> topic{};
  PubSubService<TestRequest, TestResponse> pub_sub{
      topic, [this](grpc::ClientContext* context, const TestRequest* request,
                    grpc::ClientReadReactor<TestResponse>* reactor) {
        DLOG(INFO) << "Initiating connection to server.";
        return this->client_stub->async()->server_stream(context, request, reactor);
      }};

  pub_sub.start();
  pub_sub.await();

  EXPECT_TRUE(pub_sub.status().ok());
  EXPECT_EQ(MESSAGE, topic.last_message().message());
}

TEST_F(PubSubServiceTest, CanPublishRpcStreamOverTopic) {
  for (int i = 0; i < 10; ++i) {
    using std::to_string;
    add_canned_response("Message " + to_string(i));
  }

  TestTopic<TestResponse> topic{};

  std::vector<TestResponse> responses{};
  topic.register_callback(
      [&responses](const TestResponse& message) { responses.emplace_back(message); });

  PubSubService<TestRequest, TestResponse> pub_sub{
      topic, [this](grpc::ClientContext* context, const TestRequest* request,
                    grpc::ClientReadReactor<TestResponse>* reactor) {
        return this->client_stub->async()->server_stream(context, request, reactor);
      }};

  pub_sub.start();
  pub_sub.await();

  EXPECT_TRUE(pub_sub.status().ok());
  EXPECT_EQ(service.canned_responses.size(), responses.size());
  for (uint32_t i = 0; i < responses.size(); ++i) {
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equivalent(service.canned_responses[i],
                                                                       responses[i]));
  }
}

TEST_F(PubSubServiceTest, CanPublishLongRpcStreamOverTopic) {
  for (int i = 0; i < 50; ++i) {
    using std::to_string;
    add_canned_response("Message " + to_string(i));
  }

  TestTopic<TestResponse> topic{};

  std::vector<TestResponse> responses{};
  topic.register_callback(
      [&responses](const TestResponse& message) { responses.emplace_back(message); });

  PubSubService<TestRequest, TestResponse> pub_sub{
      topic, [this](grpc::ClientContext* context, const TestRequest* request,
                    grpc::ClientReadReactor<TestResponse>* reactor) {
        return this->client_stub->async()->server_stream(context, request, reactor);
      }};

  pub_sub.start();
  pub_sub.await();

  EXPECT_TRUE(pub_sub.status().ok());
  EXPECT_EQ(service.canned_responses.size(), responses.size());
  for (uint32_t i = 0; i < responses.size(); ++i) {
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equivalent(service.canned_responses[i],
                                                                       responses[i]));
  }
}

}  // namespace tvsc::services
