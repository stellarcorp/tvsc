#include <memory>
#include <string>
#include <thread>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/radio/common/radio.grpc.pb.h"
#include "service/utility/service_runner.h"

namespace tvsc::service::radio {

class RadioServiceImpl final : public RadioService::Service {
 public:
  grpc::Status list_radios(grpc::ServerContext* context, const RadioListRequest* request,
                           Radios* reply) override {
    LOG(INFO) << "list_radios() called.";
    return grpc::Status::OK;
  }
};

}  // namespace tvsc::service::radio

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::radio;
  tvsc::service::utility::run_single_service<RadioService, RadioServiceImpl>("TVSC Radio Service");

  return 0;
}
