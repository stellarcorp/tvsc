#include "service/communications/server/mock_radio_server.h"

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "random/random.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

using namespace std::literals::chrono_literals;

grpc::Status MockRadioCommunicationsService::transmit(grpc::ServerContext* context,
                                                      const Message* request,
                                                      SuccessResult* reply) {
  LOG(INFO) << "MockRadioCommunicationsService::transmit()";
  {
    LOG(INFO) << "MockRadioCommunicationsService::transmit() -- receive_writer_queues_.size(): "
              << receive_writer_queues_.size();
    std::lock_guard<std::mutex> l(mu_);
    for (auto& entries : receive_writer_queues_) {
      // TODO(james): Make this more efficient. This approach just copies the message to the queue
      // for every writer. Better would be to share a single instance of the message across all of
      // the writers.
      LOG(INFO) << "MockRadioCommunicationsService::transmit() -- posting to writer queue";
      entries.second.push_back(*request);
    }
  }
  LOG(INFO) << "MockRadioCommunicationsService::transmit() -- notifying writers";
  receive_message_available_.notify_all();
  return grpc::Status::OK;
}

grpc::Status MockRadioCommunicationsService::receive(grpc::ServerContext* context,
                                                     const EmptyMessage* /*request*/,
                                                     grpc::ServerWriter<Message>* writer) {
  using namespace std::literals::chrono_literals;
  LOG(INFO) << "MockRadioCommunicationsService::receive()";
  std::unique_lock<std::mutex> l(mu_);
  receive_writer_queues_.emplace(writer, std::vector<Message>{});

  while (!context->IsCancelled()) {
    if (receive_message_available_.wait_for(l, 20ms,
                                            [context] { return context->IsCancelled(); })) {
      LOG(INFO) << "MockRadioCommunicationsService::receive() -- context->IsCancelled()";
      break;
    }

    auto& queue{receive_writer_queues_.at(writer)};
    for (const auto& msg : queue) {
      LOG(INFO) << "MockRadioCommunicationsService::receive() -- writing message.";
      writer->Write(msg);
    }
    queue.clear();
  }
  LOG(INFO) << "MockRadioCommunicationsService::receive() -- context->IsCancelled(): "
            << (context->IsCancelled() ? "true" : "false");

  receive_writer_queues_.erase(writer);

  LOG(INFO) << "MockRadioCommunicationsService::receive() -- exiting.";
  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

grpc::Status MockRadioCommunicationsService::monitor(
    grpc::ServerContext* context, const EmptyMessage* /*request*/,
    grpc::ServerWriter<tvsc::radio::proto::TelemetryEvent>* writer) {
  using namespace std::literals::chrono_literals;
  LOG(INFO) << "MockRadioCommunicationsService::monitor()";
  std::unique_lock<std::mutex> l(mu_);
  monitor_writer_queues_.emplace(writer, std::vector<tvsc::radio::proto::TelemetryEvent>{});

  while (!context->IsCancelled()) {
    if (monitor_event_available_.wait_for(l, 20ms, [context] { return context->IsCancelled(); })) {
      LOG(INFO) << "MockRadioCommunicationsService::monitor() -- context->IsCancelled()";
      break;
    }

    auto& queue{monitor_writer_queues_.at(writer)};
    for (const auto& msg : queue) {
      LOG(INFO) << "MockRadioCommunicationsService::monitor() -- writing telemetry event.";
      writer->Write(msg);
    }
    queue.clear();
  }
  LOG(INFO) << "MockRadioCommunicationsService::monitor() -- context->IsCancelled(): "
            << (context->IsCancelled() ? "true" : "false");

  monitor_writer_queues_.erase(writer);

  LOG(INFO) << "MockRadioCommunicationsService::monitor() -- exiting.";
  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

grpc::Status MockRadioCommunicationsService::begin_sample_broadcast(grpc::ServerContext* context,
                                                                    const EmptyMessage* /*request*/,
                                                                    EmptyMessage* /*reply*/) {
  return grpc::Status::OK;
}

grpc::Status MockRadioCommunicationsService::end_sample_broadcast(grpc::ServerContext* context,
                                                                  const EmptyMessage* /*request*/,
                                                                  EmptyMessage* /*reply*/) {
  return grpc::Status::OK;
}

}  // namespace tvsc::service::communications
