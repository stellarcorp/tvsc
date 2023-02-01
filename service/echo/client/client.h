#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/echo/common/echo.grpc.pb.h"

namespace tvsc::service::echo {

class EchoClient {
 public:
  EchoClient() : EchoClient(tvsc::discovery::service_url<Echo>()) {}

  EchoClient(const std::string& bind_addr)
      : channel_(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials())),
        stub_(Echo::NewStub(channel_)) {}

  grpc::Status call(const std::string& msg, EchoReply* reply) {
    EchoRequest request{};
    request.set_msg(msg);
    return call(request, reply);
  }

  grpc::Status call(const EchoRequest& request, EchoReply* reply) {
    grpc::ClientContext context{};
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const EchoRequest& request, EchoReply* reply) {
    grpc::Status status = stub_->echo(context, request, reply);
    LOG(INFO) << "context->peer(): " << context->peer();
    LOG(INFO) << "service config: " << channel_->GetServiceConfigJSON();
    LOG(INFO) << "EchoClient::call() -- request: " << request.DebugString()
              << ", reply: " << reply->DebugString();
    return status;
  }

 private:
  std::shared_ptr<grpc::Channel> channel_;
  std::unique_ptr<Echo::Stub> stub_;
};

}  // namespace tvsc::service::echo
