#include "service/datetime/server/server.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime.pb.h"
#include "service/datetime/common/datetime_utils.h"

namespace tvsc::service::datetime {

using namespace std::literals::chrono_literals;

std::chrono::nanoseconds default_period(TimeUnit requested_precision) {
  switch (requested_precision) {
    case TimeUnit::NANOSECOND:
    case TimeUnit::MICROSECOND:
      return 10ms;

    case TimeUnit::MILLISECOND:
    case TimeUnit::SECOND:
      return 1s;

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

grpc::Status DatetimeServiceImpl::get_datetime(grpc::ServerContext* context,
                                               const DatetimeRequest* request,
                                               DatetimeReply* reply) {
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
      return grpc::Status{grpc::StatusCode::INVALID_ARGUMENT,
                          "Unrecognized precision value in request"};
  }

  return grpc::Status::OK;
}

grpc::Status DatetimeServiceImpl::stream_datetime(grpc::ServerContext* context,
                                                  const DatetimeRequest* request,
                                                  grpc::ServerWriter<DatetimeReply>* writer) {
  using Clock = std::chrono::system_clock;

  auto response_period = compute_period(*request);

  DatetimeReply reply{};
  while (!context->IsCancelled()) {
    get_datetime(context, request, &reply);
    writer->Write(reply);

    auto timeout_duration = context->deadline() - Clock::now();
    auto sleep_duration = std::min(response_period, timeout_duration / 2);
    DLOG_EVERY_N(INFO, 1000) << "DatetimeServerImpl::stream_datetime() -- sleep_duration: "
                             << sleep_duration.count() / 1000 / 1000 << "ms";
    std::this_thread::sleep_for(sleep_duration);
  }

  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

}  // namespace tvsc::service::datetime
