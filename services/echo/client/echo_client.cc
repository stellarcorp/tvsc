#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/echo/common/echo.grpc.pb.h"

using echo::Echo;
using echo::EchoReply;
using echo::EchoRequest;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class EchoClient {
 public:
  EchoClient(std::shared_ptr<Channel> channel) : stub_(Echo::NewStub(channel)) {}

  std::string Echo(const std::string& msg) {
    EchoRequest request;
    request.set_msg(msg);

    ClientContext context;
    EchoReply reply;

    Status status = stub_->Echo(&context, request, &reply);

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

DEFINE_string(server, "localhost:50051", "Server address:port. Defaults to 'localhost:50051'.");
DEFINE_string(msg, "I am an echo!", "Message to echo. Defaults to 'I am an echo!'.");

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  EchoClient client(grpc::CreateChannel(FLAGS_server, grpc::InsecureChannelCredentials()));
  std::string reply = client.Echo(FLAGS_msg);
  std::cout << reply << "\n";

  return 0;
}
