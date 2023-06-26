#include <chrono>
#include <memory>

#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/client/client.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime.pb.h"
#include "service/datetime/server/server.h"
#include "service/utility/service_runner.h"
#include "service/utility/status_testing.h"

namespace tvsc::service::datetime {

using namespace std::literals::chrono_literals;
using Clock = std::chrono::system_clock;

class DatetimeServiceTest : public ::testing::Test {
 public:
  tvsc::service::utility::ServiceRunner service_runner{};
  DatetimeServiceImpl service{};

  DatetimeServiceTest() {
    service_runner.add_service<Datetime, DatetimeServiceImpl>("Datetime Service", service);
  }

  void SetUp() override { service_runner.start(/* advertise_services */ false); }

  void TearDown() override {
    DLOG(INFO) << "Stopping the service...";
    service_runner.stop();
    DLOG(INFO) << "Service stopped.";
  }
};

TEST_F(DatetimeServiceTest, CanCallGet) {
  DatetimeClient client{service_runner.bind_address()};

  DatetimeReply response{};
  EXPECT_OK(client.call(&response));
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessage) {
  DatetimeClient subscribed_client{service_runner.bind_address()};
  grpc::ClientContext subscribed_context{};

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context);

  DatetimeReply received_message{};
  int read_message_count{0};
  DLOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    DLOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime_ms(), 0);

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanSubscribeAndGetManyMessages) {
  constexpr int NUM_MESSAGES{20};
  DatetimeClient subscribed_client{service_runner.bind_address()};
  grpc::ClientContext subscribed_context{};

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context);

  DatetimeReply received_message{};
  int read_message_count{0};
  while (reader->Read(&received_message)) {
    ++read_message_count;
    EXPECT_GT(received_message.datetime_ms(), 0);

    if (read_message_count == NUM_MESSAGES) {
      subscribed_context.TryCancel();
    }
  }

  EXPECT_EQ(NUM_MESSAGES, read_message_count);
}

}  // namespace tvsc::service::datetime
