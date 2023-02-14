#include <csignal>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include "discovery/service_advertiser.h"
#include "discovery/service_types.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "service/radio/common/radio.grpc.pb.h"

namespace tvsc::service::radio {

class RadioServiceImpl final : public RadioService::Service {
 public:
  grpc::Status list_radios(grpc::ServerContext* context, const RadioListRequest* request,
                           Radios* reply) override {
    LOG(INFO) << "list_radios() called.";
    return grpc::Status::OK;
  }
};

void run_server() {
  tvsc::service::radio::RadioServiceImpl service{};

  grpc::EnableDefaultHealthCheckService(true);
  grpc::ServerBuilder builder;

  int port{0};
  builder.AddListeningPort("dns:///[::]:0", grpc::InsecureServerCredentials(), &port);

  builder.RegisterService(service);

  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

  tvsc::discovery::ServiceAdvertiser advertiser{};
  advertiser.advertise_service(
      "TVSC Radio Service", tvsc::discovery::generate_service_type<RadioService>(), "local", port);

  LOG(INFO) << "Server listening on port " << port;
  server->Wait();
}

}  // namespace tvsc::service::radio

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::radio::run_server();

  return 0;
}
