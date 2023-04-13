#include "service/communications/server/server.h"

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <thread>

#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "radio/packet.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

using namespace std::literals::chrono_literals;

void CommunicationsServiceImpl::post_received_packet(const tvsc::radio::Packet& packet) {
  DLOG(INFO) << "CommunicationsServerImpl::post_received_packet()";
  {
    Message message{};
    message.ParseFromString(std::string(packet.payload().as_string_view(packet.payload_length())));

    std::lock_guard<std::mutex> l(mu_);
    for (auto& writer_queue : writer_queues_) {
      // TODO(james): Make this more efficient. This approach just copies the message to the queue
      // for every writer. Better would be to share a single instance of the message across all of
      // the writers.
      DLOG(INFO) << "CommunicationsServerImpl::post_received_packet() -- posting to writer queue";
      writer_queue.second.push_back(message);
    }
  }
  DLOG(INFO) << "CommunicationsServerImpl::post_received_packet() -- notifying writers";
  cv_.notify_all();
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
  using namespace std::literals::chrono_literals;
  DLOG(INFO) << "CommunicationsServiceImpl::receive()";
  std::unique_lock<std::mutex> l(mu_);
  writer_queues_.emplace(writer, std::vector<Message>{});

  while (!context->IsCancelled()) {
    if (cv_.wait_for(l, 20ms, [context] { return context->IsCancelled(); })) {
      DLOG(INFO) << "CommunicationsServiceImpl::receive() -- context->IsCancelled()";
      break;
    }

    auto& queue{writer_queues_.at(writer)};
    for (const auto& msg : queue) {
      DLOG(INFO) << "CommunicationsServiceImpl::receive() -- writing message.";
      writer->Write(msg);
    }
    queue.clear();
  }
  DLOG(INFO) << "CommunicationsServiceImpl::receive() -- context->IsCancelled(): "
             << (context->IsCancelled() ? "true" : "false");

  writer_queues_.erase(writer);

  DLOG(INFO) << "CommunicationsServiceImpl::receive() -- exiting.";
  // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
  return grpc::Status::OK;
}

}  // namespace tvsc::service::communications
