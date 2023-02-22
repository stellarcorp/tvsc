#include <memory>
#include <string>
#include <thread>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "radio/radio.pb.h"
#include "service/communications/common/communications.grpc.pb.h"
#include "service/utility/service_runner.h"

namespace tvsc::service::communications {

class CommunicationsServiceImpl final : public CommunicationsService::Service {
 public:
  grpc::Status list_radios(grpc::ServerContext* context, const RadioListRequest* request,
                           tvsc::radio::Radios* reply) override {
    LOG(INFO) << "list_radios() called.";
    return grpc::Status::OK;
  }
};

}  // namespace tvsc::service::communications

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::communications;
  tvsc::service::utility::run_single_service<CommunicationsService, CommunicationsServiceImpl>(
      "TVSC Communications Service");

  return 0;
}
