#include "service/communications/server/server.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

using namespace std::literals::chrono_literals;

template <typename Clock, typename Rep, typename Period>
void sleep_while_checking_for_cancel(std::chrono::duration<Rep, Period> sleep_duration,
                                     grpc::ServerContext& context) {
  const std::chrono::time_point<Clock> wakeup_time{Clock::now() + sleep_duration};
  // We change the meaning of the sleep_duration variable here to avoid allocating a new duration.
  // It used to mean the total amount of time for this method to sleep. Now it means the individual
  // interval to sleep between checking if the context has been cancelled.
  sleep_duration = std::min(std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(50ms),
                            sleep_duration);
  while (!context.IsCancelled() && wakeup_time > Clock::now()) {
    std::this_thread::sleep_for(sleep_duration);
  }
}

grpc::Status CommunicationsServiceImpl::list_radios(grpc::ServerContext* context,
                                                    const EmptyMessage* request, Radios* reply) {
  LOG(INFO) << "list_radios() called.";
  return grpc::Status::OK;
}

grpc::Status CommunicationsServiceImpl::transmit(grpc::ServerContext* context,
                                                 const Message* request, SuccessResult* reply) {
  LOG(INFO) << "transmit() called. Message: " << request->message();
  return grpc::Status::OK;
}

grpc::Status CommunicationsServiceImpl::receive(grpc::ServerContext* context,
                                                const EmptyMessage* /*request*/,
                                                grpc::ServerWriter<Message>* writer) {
  Message reply{};
  while (!context->IsCancelled()) {
    // For now, just spit out the current time as if it had been received over a radio.
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::time_t as_time_t{std::chrono::system_clock::to_time_t(now)};
    std::stringstream buffer{};
    buffer << std::put_time(std::localtime(&as_time_t), "%F %T") << "\n";
    reply.set_message(buffer.str());
    writer->Write(reply);

    sleep_while_checking_for_cancel<std::chrono::system_clock>(1000ms, *context);
  }

  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

}  // namespace tvsc::service::communications
