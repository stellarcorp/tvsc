#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "services/radio/common/radio.grpc.pb.h"
#include "services/radio/common/radio_service_location.h"

namespace tvsc::service::radio {

class RadioClient {
 public:
  RadioClient() : RadioClient(get_radio_service_socket_address()) {}

  RadioClient(const std::string& bind_addr)
      : stub_(Radio::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  grpc::Status call(const std::string& msg, RadioReply* reply) {
    grpc::ClientContext context{};
    RadioRequest request{};
    request.set_msg(msg);
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const RadioRequest& request, RadioReply* reply) {
    return stub_->radio(context, request, reply);
  }

 private:
  std::unique_ptr<Radio::Stub> stub_;
};

}  // namespace tvsc::service::radio
