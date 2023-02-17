#include "gflags/gflags.h"
#include "glog/logging.h"
#include "service/hello/common/hello.grpc.pb.h"
#include "service/hello/server/server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::hello;
  tvsc::service::utility::run_single_service<Hello, HelloServiceImpl>("TVSC Greeter Service");

  return 0;
}
