#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "services/configuration/service_types.h"
#include "services/echo/common/echo.grpc.pb.h"

namespace tvsc::service::echo {

class EchoClient {
 public:
  EchoClient() : EchoClient(tvsc::service::configuration::default_bind_address<Echo>()) {}

  EchoClient(const std::string& bind_addr)
      : stub_(Echo::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  grpc::Status call(const std::string& msg, EchoReply* reply) {
    grpc::ClientContext context{};
    EchoRequest request{};
    request.set_msg(msg);
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const EchoRequest& request, EchoReply* reply) {
    return stub_->echo(context, request, reply);
  }

 private:
  std::unique_ptr<Echo::Stub> stub_;
};

}  // namespace tvsc::service::echo
