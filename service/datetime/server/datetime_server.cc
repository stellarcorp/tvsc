#include "base/initializer.h"
#include "service/datetime/common/datetime.grpc.pb.h"
#include "service/datetime/server/server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  tvsc::initialize(&argc, &argv);
  using namespace tvsc::service::datetime;
  tvsc::service::utility::run_single_service<Datetime, DatetimeServiceImpl>(
      "TVSC Datetime Service");

  return 0;
}
