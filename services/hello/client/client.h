#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "services/hello/common/hello.grpc.pb.h"
#include "services/hello/common/hello_service_location.h"

namespace tvsc::service::hello {

class HelloClient {
 public:
  HelloClient() : HelloClient(get_hello_service_socket_address()) {}

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
