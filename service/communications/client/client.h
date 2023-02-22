#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "radio/radio.pb.h"
#include "service/communications/common/communications.grpc.pb.h"

namespace tvsc::service::communications {

class CommunicationsClient {
 public:
  CommunicationsClient()
      : CommunicationsClient(tvsc::discovery::service_url<CommunicationsService>()) {}

  CommunicationsClient(const std::string& bind_addr)
      : stub_(CommunicationsService::NewStub(
            grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  grpc::Status list_radios(tvsc::radio::Radios* reply) {
    grpc::ClientContext context{};
    return stub_->list_radios(&context, RadioListRequest{}, reply);
  }

 private:
  std::unique_ptr<CommunicationsService::Stub> stub_;
};

}  // namespace tvsc::service::communications
