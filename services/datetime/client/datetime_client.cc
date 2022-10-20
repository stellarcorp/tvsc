#include <iostream>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/datetime/client/client.h"
#include "services/datetime/common/datetime.grpc.pb.h"
#include "services/datetime/common/datetime_service_location.h"

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

  std::cout << tvsc::service::datetime::get_datetime() << " ms\n";

  return 0;
}
