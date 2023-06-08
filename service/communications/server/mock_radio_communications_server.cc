#include "gflags/gflags.h"
#include "glog/logging.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/communications/server/mock_radio_server.h"
#include "service/utility/service_runner.h"

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::communications;
  tvsc::service::utility::run_single_service<CommunicationsService, MockRadioCommunicationsService>(
      "Mock Radio Communications Service");

  return 0;
}
