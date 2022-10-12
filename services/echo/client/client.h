#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "services/echo/common/echo.grpc.pb.h"

namespace tvsc::service::echo {

class EchoClient {
 public:
  EchoClient(const std::string& bind_addr)
      : stub_(Echo::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  grpc::Status call(const std::string& message, EchoReply* reply) {
    grpc::ClientContext context{};
    EchoRequest request{};
    request.set_msg(message);
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const EchoRequest& request, EchoReply* reply) {
    return stub_->echo(context, request, reply);
  }

 private:
  std::unique_ptr<Echo::Stub> stub_;
};

}  // namespace tvsc::service::echo
