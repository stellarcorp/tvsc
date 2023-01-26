#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "grpcpp/grpcpp.h"
#include "service/hello/common/hello.grpc.pb.h"

namespace tvsc::service::hello {

class HelloClient {
 public:
  HelloClient() : HelloClient(tvsc::discovery::default_bind_address<Hello>()) {}

  HelloClient(const std::string& bind_addr)
      : stub_(Hello::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  grpc::Status call(const std::string& name, HelloReply* reply) {
    grpc::ClientContext context{};
    HelloRequest request{};
    request.set_name(name);
    return call(&context, request, reply);
  }

  grpc::Status call(grpc::ClientContext* context, const HelloRequest& request, HelloReply* reply) {
    return stub_->say_hello(context, request, reply);
  }

 private:
  std::unique_ptr<Hello::Stub> stub_;
};

}  // namespace tvsc::service::hello
