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

  void SetUp() override { service_runner.start(); }

  void TearDown() override { service_runner.stop(); }
};

TEST_F(DatetimeServiceTest, CanCallGet) {
  DatetimeClient client{service_runner.bind_address()};

  DatetimeReply response{};
  EXPECT_OK(client.call(TimeUnit::SECOND, &response));
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessage) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::NANOSECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::NANOSECOND, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageWithRepeatPeriod) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::YEAR);
  request.set_period_count(10);
  request.set_period_unit(TimeUnit::MILLISECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::YEAR, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageNanosecondPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::NANOSECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::NANOSECOND, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageMicrosecondPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::MICROSECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::MICROSECOND, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageMillisecondPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::MILLISECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::MILLISECOND, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageSecondPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::SECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::SECOND, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageMinutePrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::MINUTE);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::MINUTE, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageHourPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::HOUR);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::HOUR, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageDayPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::DAY);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::DAY, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageWeekPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::WEEK);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::WEEK, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageMonthPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::MONTH);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::MONTH, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanRequestStreamAndGetSingleMessageYearPrecision) {
  DatetimeRequest request{};
  request.set_precision(TimeUnit::YEAR);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::YEAR, received_message.unit());

    subscribed_context.TryCancel();
  }
  EXPECT_EQ(1, read_message_count);
}

TEST_F(DatetimeServiceTest, CanSubscribeAndGetManyMessages) {
  constexpr int NUM_MESSAGES{20};
  DatetimeRequest request{};
  request.set_precision(TimeUnit::YEAR);
  request.set_period_count(1);
  request.set_period_unit(TimeUnit::MILLISECOND);

  DatetimeClient subscribed_client{service_runner.bind_address()};

  grpc::ClientContext subscribed_context{};
  // 100ms to get a message.
  subscribed_context.set_deadline(Clock::now() + 100ms);

  std::unique_ptr<grpc::ClientReaderInterface<DatetimeReply>> reader{};
  reader = subscribed_client.stream(&subscribed_context, request);

  DatetimeReply received_message{};
  int read_message_count{0};
  LOG(INFO) << "Reading messages from reader";
  while (reader->Read(&received_message)) {
    LOG(INFO) << "Read message! -- received_message: " << received_message.DebugString();
    ++read_message_count;
    EXPECT_GT(received_message.datetime(), 0);
    EXPECT_EQ(TimeUnit::YEAR, received_message.unit());

    if (read_message_count == NUM_MESSAGES) {
      subscribed_context.TryCancel();
    }
  }

  EXPECT_EQ(NUM_MESSAGES, read_message_count);
}

}  // namespace tvsc::service::datetime
