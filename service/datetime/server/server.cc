#include "service/datetime/server/server.h"

#include <algorithm>
#include <chrono>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/common/datetime.pb.h"

namespace tvsc::service::datetime {

using namespace std::literals::chrono_literals;

template <typename Clock>
void sleep_while_checking_for_cancel(std::chrono::nanoseconds sleep_duration,
                                     grpc::ServerContext& context) {
  const std::chrono::time_point<Clock> wakeup_time{Clock::now() + sleep_duration};
  // We change the meaning of the sleep_duration variable here to avoid allocating a new duration.
  // It used to mean the total amount of time for this method to sleep. Now it means the individual
  // interval to sleep between checking if the context has been cancelled.
  sleep_duration =
      std::min(std::chrono::duration_cast<std::chrono::nanoseconds>(50ms), sleep_duration);
  while (!context.IsCancelled() && wakeup_time > Clock::now()) {
    std::this_thread::sleep_for(sleep_duration);
  }
}

grpc::Status DatetimeServiceImpl::get_datetime(grpc::ServerContext* context,
                                               const DatetimeRequest* request,
                                               DatetimeReply* reply) {
  using Clock = std::chrono::system_clock;

  Clock::duration since_epoch = Clock::now().time_since_epoch();
  reply->set_datetime_ms(
      std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch).count());

  return grpc::Status::OK;
}

grpc::Status DatetimeServiceImpl::stream_datetime(grpc::ServerContext* context,
                                                  const DatetimeRequest* request,
                                                  grpc::ServerWriter<DatetimeReply>* writer) {
  using Clock = std::chrono::system_clock;

  auto response_period = std::chrono::duration_cast<std::chrono::nanoseconds>(25ms);

  DatetimeReply reply{};
  while (!context->IsCancelled()) {
    get_datetime(context, request, &reply);
    writer->Write(reply);

    auto timeout_duration = context->deadline() - Clock::now();
    auto sleep_duration = std::min(response_period, timeout_duration / 2);
    DLOG_EVERY_N(INFO, 1000) << "DatetimeServerImpl::stream_datetime() -- sleep_duration: "
                             << sleep_duration.count() / 1000 / 1000 << "ms";

    sleep_while_checking_for_cancel<Clock>(sleep_duration, *context);
  }

  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

}  // namespace tvsc::service::datetime
