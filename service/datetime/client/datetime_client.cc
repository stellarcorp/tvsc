#include <iostream>

#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/client/client.h"
#include "service/datetime/common/datetime.grpc.pb.h"

namespace tvsc::service::datetime {

int64_t get_datetime() {
  DatetimeClient client{};
  DatetimeReply reply{};
  grpc::Status status = client.call(&reply);
  if (status.ok()) {
    return reply.datetime();
  } else {
    LOG(ERROR) << "RPC failed -- " << status.error_code() << ": " << status.error_message();
    return 0;
  }
}

}  // namespace tvsc::service::datetime

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::register_mdns_grpc_resolver();

  std::cout << tvsc::service::datetime::get_datetime() << " ms\n";

  return 0;
}
