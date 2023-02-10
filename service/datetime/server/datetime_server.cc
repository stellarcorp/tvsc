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

using namespace std::literals::chrono_literals;

std::chrono::nanoseconds default_period(TimeUnit requested_precision) {
  switch (requested_precision) {
    case TimeUnit::NANOSECOND:
    case TimeUnit::MICROSECOND:
      return 100us;

    case TimeUnit::MILLISECOND:
      return 1ms;

    case TimeUnit::SECOND:
      return 100ms;
    case TimeUnit::MINUTE:
      return 5s;

    case TimeUnit::HOUR:
    case TimeUnit::DAY:
    case TimeUnit::WEEK:
    case TimeUnit::YEAR:
    case TimeUnit::MONTH:
      // Keep a heartbeat that can be detected at reasonable debugging scales. This is slow
      // enough that it shouldn't be a meaningful load anyway.
      return 10s;
      break;

    default:
      LOG(WARNING) << "Unrecognized precision value in request. Attempting to choose a reasonable "
                      "default value.";
      // Since this is likely coming from a client request, it's possible the client is just at a
      // different version of the proto than we are. Try to accomodate.
      return 10s;
  }
}

std::chrono::nanoseconds compute_period(const DatetimeRequest& request) {
  if (request.period_count() == 0) {
    // If the period is empty, use the default period for the requested precision.
    return default_period(request.precision());
  }

  return as_duration(request.period_count(), request.period_unit());
}

class DatetimeServiceImpl final : public Datetime::Service {
  Status get_datetime(ServerContext* context, const DatetimeRequest* request,
                      DatetimeReply* reply) override {
    using Clock = std::chrono::system_clock;

    Clock::duration since_epoch = Clock::now().time_since_epoch();
    reply->set_unit(request->precision());
    switch (request->precision()) {
      case TimeUnit::NANOSECOND:
        reply->set_datetime(
            std::chrono::duration_cast<std::chrono::nanoseconds>(since_epoch).count());
        break;

      case TimeUnit::MICROSECOND:
        reply->set_datetime(
            std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count());
        break;

      case TimeUnit::MILLISECOND:
        reply->set_datetime(
            std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch).count());
        break;

      case TimeUnit::SECOND:
        reply->set_datetime(std::chrono::duration_cast<std::chrono::seconds>(since_epoch).count());
        break;

      case TimeUnit::MINUTE:
        reply->set_datetime(std::chrono::duration_cast<std::chrono::minutes>(since_epoch).count());
        break;

      case TimeUnit::HOUR:
        reply->set_datetime(std::chrono::duration_cast<std::chrono::hours>(since_epoch).count());
        break;

      case TimeUnit::DAY:
        reply->set_datetime(std::chrono::duration_cast<DayDuration>(since_epoch).count());
        break;

      case TimeUnit::WEEK:
        reply->set_datetime(std::chrono::duration_cast<WeekDuration>(since_epoch).count());
        break;

      case TimeUnit::MONTH:
        reply->set_datetime(std::chrono::duration_cast<MonthDuration>(since_epoch).count());
        break;

      case TimeUnit::YEAR:
        reply->set_datetime(std::chrono::duration_cast<YearDuration>(since_epoch).count());
        break;

      default:
        LOG(WARNING) << "Unrecognized precision value in request";
        return Status{StatusCode::INVALID_ARGUMENT, "Unrecognized precision value in request"};
    }

    return Status::OK;
  }

  Status stream_datetime(ServerContext* context, const DatetimeRequest* request,
                         grpc::ServerWriter<DatetimeReply>* writer) override {
    using Clock = std::chrono::system_clock;

    auto response_period = compute_period(*request);

    DatetimeReply reply{};
    while (!context->IsCancelled()) {
      get_datetime(context, request, &reply);
      writer->Write(reply);

      auto timeout_duration = context->deadline() - Clock::now();
      auto sleep_duration = std::min(response_period, timeout_duration / 2);
      std::this_thread::sleep_for(sleep_duration);
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
