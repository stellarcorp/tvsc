#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "services/radio/common/radio.grpc.pb.h"

namespace tvsc::service::radio {

class RadioClient {
 public:
  RadioClient() : RadioClient(tvsc::discovery::default_bind_address<RadioService>()) {}

  RadioClient(const std::string& bind_addr)
      : stub_(RadioService::NewStub(
            grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  grpc::Status list_radios(Radios* reply) {
    grpc::ClientContext context{};
    return stub_->list_radios(&context, RadioListRequest{}, reply);
  }

 private:
  std::unique_ptr<RadioService::Stub> stub_;
};

}  // namespace tvsc::service::radio
