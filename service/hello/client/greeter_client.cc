#include <iostream>
#include <string>

#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/hello/client/client.h"
#include "service/hello/common/hello.grpc.pb.h"

namespace tvsc::service::hello {

std::string get_hello_message(const std::string& name) {
  using std::to_string;
  HelloClient client{};
  HelloReply reply{};
  grpc::Status status = client.call(name, &reply);
  if (status.ok()) {
    return reply.msg();
  } else {
    LOG(ERROR) << status.error_code() << ": " << status.error_message();
    return std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " +
           status.error_message();
  }
}

}  // namespace tvsc::service::hello

DEFINE_string(name, "world", "Name of person to greet. Defaults to 'world'.");

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::register_mdns_grpc_resolver();

  std::cout << tvsc::service::hello::get_hello_message(FLAGS_name) << "\n";

  return 0;
}