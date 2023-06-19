#include <iostream>
#include <memory>
#include <string>

#include "base/initializer.h"
#include "discovery/service_resolver.h"
#include "grpcpp/grpcpp.h"
#include "service/echo/client/client.h"
#include "service/echo/common/echo.grpc.pb.h"

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
  tvsc::initialize(&argc, &argv);

  tvsc::discovery::register_mdns_grpc_resolver();

  std::cout << tvsc::service::echo::echo_message(FLAGS_msg) << "\n";

  return 0;
}
