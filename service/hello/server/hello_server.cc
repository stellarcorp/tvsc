#include "base/initializer.h"
#include "service/hello/common/hello.grpc.pb.h"
#include "service/hello/server/server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  tvsc::initialize(&argc, &argv);
  using namespace tvsc::service::hello;
  tvsc::service::utility::run_single_service<Hello, HelloServiceImpl>("TVSC Greeter Service");

  return 0;
}
