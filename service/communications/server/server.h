#pragma once

#include <condition_variable>
#include <map>
#include <mutex>

#include "grpcpp/grpcpp.h"
#include "radio/packet.h"
#include "radio/packet_queue.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class CommunicationsServiceImpl final : public CommunicationsService::Service {
 private:
  std::mutex mu_{};
  std::condition_variable cv_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> writer_queues_{};

  void post_received_packet(const tvsc::radio::Packet& packet);

 public:
  grpc::Status list_radios(grpc::ServerContext* context, const EmptyMessage* request,
                           Radios* reply) override;

  grpc::Status transmit(grpc::ServerContext* context, const Message* request,
                        SuccessResult* reply) override;

  grpc::Status receive(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<Message>* writer) override;
};

}  // namespace tvsc::service::communications
