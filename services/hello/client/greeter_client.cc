#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/hello/common/hello.grpc.pb.h"
#include "services/hello/common/hello_service_location.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace tvsc::service::hello {

class HelloClient {
 public:
  HelloClient(const std::string& bind_addr)
      : stub_(Hello::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  std::string say_hello(const std::string& name) {
    HelloRequest request;
    request.set_name(name);

    ClientContext context;
    HelloReply reply;

    Status status = stub_->say_hello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.msg();
    } else {
      LOG(ERROR) << status.error_code() << ": " << status.error_message();
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Hello::Stub> stub_;
};

std::string get_hello_message(const std::string& name) {
  HelloClient client(get_hello_service_bind_addr());
  return client.say_hello(name);
}

}  // namespace tvsc::service::hello

DEFINE_string(name, "world", "Name of person to greet. Defaults to 'world'.");

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::cout << tvsc::service::hello::get_hello_message(FLAGS_name) << "\n";

  return 0;
}
