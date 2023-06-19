#include "base/initializer.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/server/server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  tvsc::initialize(&argc, &argv);
  using namespace tvsc::service::communications;
  tvsc::service::utility::run_single_service<CommunicationsService, CommunicationsServiceImpl>(
      "TVSC Communications Service");

  return 0;
}
