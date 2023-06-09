#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "service/communications/common/communications.grpc.pb.h"

namespace tvsc::service::communications {

class CommunicationsClient {
 public:
  CommunicationsClient()
      : CommunicationsClient(tvsc::discovery::service_url<CommunicationsService>()) {}

  CommunicationsClient(const std::string& bind_addr)
      : stub_(CommunicationsService::NewStub(
            grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  std::unique_ptr<grpc::ClientReaderInterface<Message>> receive(grpc::ClientContext* context) {
    return stub_->receive(context, EmptyMessage{});
  }

  void receive(grpc::ClientContext* context, grpc::ClientReadReactor<Message>* reactor) {
    EmptyMessage request{};
    stub_->async()->receive(context, &request, reactor);
  }

  grpc::Status transmit(const std::string& message, SuccessResult* reply) {
    grpc::ClientContext context{};
    Message m{};
    m.set_message(message);
    return stub_->transmit(&context, m, reply);
  }

  grpc::Status transmit(const Message& message, SuccessResult* reply) {
    grpc::ClientContext context{};
    return stub_->transmit(&context, message, reply);
  }

 private:
  std::unique_ptr<CommunicationsService::Stub> stub_;
};

}  // namespace tvsc::service::communications
