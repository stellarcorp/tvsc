#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/datetime/common/datetime.grpc.pb.h"
#include "services/datetime/common/datetime_service_location.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

namespace tvsc::service::datetime {

class DatetimeServiceImpl final : public Datetime::Service {
  Status get_datetime(ServerContext* context, const DatetimeRequest* request, DatetimeReply* reply) override {
    using Clock = std::chrono::system_clock;

    constexpr int32_t SECONDS_IN_DAY{24 * 60 * 60};
    constexpr int32_t SECONDS_IN_WEEK{7 * SECONDS_IN_DAY};
    // Days in a year is the average length of the Gregorian year, per
    // https://en.cppreference.com/w/cpp/chrono/duration
    constexpr int32_t SECONDS_IN_YEAR{static_cast<int32_t>(365.2425 * SECONDS_IN_DAY)};
    // Per https://en.cppreference.com/w/cpp/chrono/duration
    constexpr int32_t SECONDS_IN_MONTH{static_cast<int32_t>(SECONDS_IN_YEAR / 12)};

    Clock::duration since_epoch = Clock::now().time_since_epoch();
    int64_t count{0};
    switch (request->precision()) {
      case DatetimeRequest::NANOSECOND:
        count = std::chrono::duration_cast<std::chrono::nanoseconds>(since_epoch).count();
        break;

      case DatetimeRequest::MICROSECOND:
        count = std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count();
        break;

      case DatetimeRequest::MILLISECOND:
        count = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch).count();
        break;

      case DatetimeRequest::SECOND:
        count = std::chrono::duration_cast<std::chrono::seconds>(since_epoch).count();
        break;

      case DatetimeRequest::MINUTE:
        count = std::chrono::duration_cast<std::chrono::minutes>(since_epoch).count();
        break;

      case DatetimeRequest::HOUR:
        count = std::chrono::duration_cast<std::chrono::hours>(since_epoch).count();
        break;

      case DatetimeRequest::DAY:
        using DayDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_DAY>>;
        count = std::chrono::duration_cast<DayDuration>(since_epoch).count();
        break;

      case DatetimeRequest::WEEK:
        using WeekDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_WEEK>>;
        count = std::chrono::duration_cast<WeekDuration>(since_epoch).count();
        break;

      case DatetimeRequest::YEAR:
        using YearDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_YEAR>>;
        count = std::chrono::duration_cast<YearDuration>(since_epoch).count();
        break;

      case DatetimeRequest::MONTH:
        using MonthDuration = std::chrono::duration<int32_t, std::ratio<SECONDS_IN_MONTH>>;
        count = std::chrono::duration_cast<MonthDuration>(since_epoch).count();
        break;

      default:
        LOG(WARNING) << "Unrecognized precision value in request";
        return Status{StatusCode::INVALID_ARGUMENT, "Unrecognized precision value in request"};
    }
    reply->set_datetime(count);
    return Status::OK;
  }
};

void run_server() {
  DatetimeServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;

  const std::string bind_addr{get_datetime_service_socket_address()};
  builder.AddListeningPort(bind_addr, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << bind_addr;

  server->Wait();
}

}  // namespace tvsc::service::datetime

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::datetime::run_server();

  return 0;
}
