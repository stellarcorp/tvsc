#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "services/echo/common/echo.grpc.pb.h"
#include "services/echo/common/echo_service_location.h"

namespace tvsc::service::echo {

class EchoClient {
 public:
  EchoClient() : EchoClient(get_echo_service_socket_address()) {}

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
