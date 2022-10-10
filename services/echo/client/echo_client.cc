#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/echo/common/echo.grpc.pb.h"
#include "services/echo/common/echo_service_location.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace tvsc::service::echo {

class EchoClient {
 public:
  EchoClient(const std::string& bind_addr)
      : stub_(Echo::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {}

  std::string echo(const std::string& msg) {
    EchoRequest request;
    request.set_msg(msg);

    ClientContext context;
    EchoReply reply;

    Status status = stub_->echo(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.msg();
    } else {
      LOG(ERROR) << status.error_code() << ": " << status.error_message();
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Echo::Stub> stub_;
};

std::string echo_message(const std::string& msg) {
  EchoClient client(get_echo_service_bind_addr());
  return client.echo(msg);
}

}  // namespace tvsc::service::echo

DEFINE_string(msg, "I am an echo!", "Message to echo. Defaults to 'I am an echo!'.");

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::cout << tvsc::service::echo::echo_message(FLAGS_msg) << "\n";

  return 0;
}
