#include <iostream>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/echo/client/client.h"
#include "services/echo/common/echo.grpc.pb.h"

namespace tvsc::service::echo {

std::string echo_message(const std::string& msg) {
  EchoClient client{};
  EchoReply reply{};
  grpc::Status status = client.call(msg, &reply);
  if (status.ok()) {
    return reply.msg();
  } else {
    LOG(ERROR) << status.error_code() << ": " << status.error_message();
    return std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " +
           status.error_message();
  }
}

}  // namespace tvsc::service::echo

DEFINE_string(msg, "I am an echo!", "Message to echo. Defaults to 'I am an echo!'.");

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::cout << tvsc::service::echo::echo_message(FLAGS_msg) << "\n";

  return 0;
}
