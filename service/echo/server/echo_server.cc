#include "base/initializer.h"
#include "service/echo/common/echo.grpc.pb.h"
#include "service/echo/server/server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  tvsc::initialize(&argc, &argv);

  using namespace tvsc::service::echo;
  tvsc::service::utility::run_single_service<Echo, EchoServiceImpl>("TVSC Echo Service");

  return 0;
}
