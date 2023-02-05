#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "discovery/service_advertiser.h"
#include "discovery/service_types.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime_utils.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

namespace tvsc::service::datetime {

class DatetimeServiceImpl final : public Datetime::Service {
  Status get_datetime(ServerContext* context, const DatetimeRequest* request,
                      DatetimeReply* reply) override {
    using Clock = std::chrono::system_clock;

    constexpr int32_t SECONDS_IN_DAY{24 * 60 * 60};
    constexpr int32_t SECONDS_IN_WEEK{7 * SECONDS_IN_DAY};
    constexpr int32_t SECONDS_IN_YEAR{static_cast<int32_t>(DAYS_IN_YEAR * SECONDS_IN_DAY)};

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

  Status stream_datetime(ServerContext* context, const DatetimeRequest* request,
                         grpc::ServerWriter<DatetimeReply>* writer) override {
    using Clock = std::chrono::system_clock;
    using namespace std::literals::chrono_literals;

    std::chrono::nanoseconds requested_precision{as_duration(request->precision())};

    // Determine an appropriate rate to send responses. If the requested precision is very fast (us
    // or ns), then we won't even call the sleep function.
    // Note: on fast machines, we see timings of approximately 5us per stream reply.
    auto max_sleep_time_for_precision{requested_precision};
    switch (request->precision()) {
      case DatetimeRequest::NANOSECOND:
      case DatetimeRequest::MICROSECOND:
        // Too fast to meet precision anyway, so don't even sleep.
        max_sleep_time_for_precision = 0 * requested_precision;
        break;

      case DatetimeRequest::MILLISECOND:
      case DatetimeRequest::SECOND:
      case DatetimeRequest::MINUTE:
        max_sleep_time_for_precision = requested_precision / 10;
        break;

      case DatetimeRequest::HOUR:
      case DatetimeRequest::DAY:
      case DatetimeRequest::WEEK:
      case DatetimeRequest::YEAR:
      case DatetimeRequest::MONTH:
        // Keep a heartbeat that can be detected at reasonable debugging scales. This is slow
        // enough that it shouldn't be a meaningful load anyway.
        max_sleep_time_for_precision = 10s;
        break;

      default:
        LOG(WARNING) << "Unrecognized precision value in request";
        return Status{StatusCode::INVALID_ARGUMENT, "Unrecognized precision value in request"};
    }

    DatetimeReply reply{};
    while (!context->IsCancelled()) {
      get_datetime(context, request, &reply);
      writer->Write(reply);

      if (max_sleep_time_for_precision != std::chrono::nanoseconds::zero()) {
        auto timeout_duration = context->deadline() - Clock::now();
        auto sleep_duration = std::min(max_sleep_time_for_precision, timeout_duration / 2);
        std::this_thread::sleep_for(sleep_duration);
      }
    }

    // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
    return Status::OK;
  }
};

void run_server() {
  DatetimeServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;

  int port{0};
  builder.AddListeningPort("dns:///[::]:0", grpc::InsecureServerCredentials(), &port);

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  tvsc::discovery::ServiceAdvertiser advertiser{};
  advertiser.advertise_service("TVSC Datetime Service",
                               tvsc::discovery::generate_service_type<Datetime>(), "local", port);

  LOG(INFO) << "Server listening on port " << port;
  server->Wait();
}

}  // namespace tvsc::service::datetime

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::datetime::run_server();

  return 0;
}
