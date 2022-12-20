#include <iostream>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/radio/client/client.h"
#include "services/radio/common/radio.grpc.pb.h"
#include "services/radio/common/radio_service_location.h"

namespace tvsc::service::radio {

std::string radio_message(const std::string& msg) {
  RadioClient client(get_radio_service_socket_address());
  EchoReply reply{};
  grpc::Status status = client.call(msg, &reply);
  if (status.ok()) {
    return reply.msg();
  } else {
    LOG(ERROR) << status.error_code() << ": " << status.error_message();
    return std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " + status.error_message();
  }
}

}  // namespace tvsc::service::radio

DEFINE_string(msg, "I am a radio!", "Message to radio. Defaults to 'I am a radio!'.");

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::cout << tvsc::service::radio::radio_message(FLAGS_msg) << "\n";

  return 0;
}
