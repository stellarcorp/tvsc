#pragma once

#include <condition_variable>
#include <future>
#include <map>
#include <memory>
#include <mutex>

#include "grpcpp/grpcpp.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

class MockRadioCommunicationsService final : public CommunicationsService::Service {
 private:
  std::mutex mu_{};
  std::condition_variable cv_{};

  std::map<grpc::ServerWriter<Message>*, std::vector<Message>> writer_queues_{};

 public:
  grpc::Status transmit(grpc::ServerContext* context, const Message* request,
                        SuccessResult* reply) override;

  grpc::Status receive(grpc::ServerContext* context, const EmptyMessage* request,
                       grpc::ServerWriter<Message>* writer) override;
};

}  // namespace tvsc::service::communications
