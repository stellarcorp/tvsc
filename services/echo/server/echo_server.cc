#include <iostream>
#include <memory>
#include <string>

#include "discovery/service_advertiser.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/configuration/service_types.h"
#include "services/echo/common/echo.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace tvsc::service::echo {

class EchoServiceImpl final : public Echo::Service {
  Status echo(ServerContext* context, const EchoRequest* request, EchoReply* reply) override {
    const std::string& msg{request->msg()};
    reply->set_msg(msg);
    LOG(INFO) << "Received msg: '" << msg << "'";
    return Status::OK;
  }
};

void run_server() {
  EchoServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;

  int port{0};
  builder.AddListeningPort("dns:///[::]:50035", grpc::InsecureServerCredentials(), &port);

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  tvsc::discovery::ServiceAdvertiser advertiser{};
  advertiser.advertise_service(
      "TVSC Echo Service",
      tvsc::service::configuration::generate_service_type<Echo>(), "local", port,
      [&server](tvsc::discovery::AdvertisementResult result) {
        if (result != tvsc::discovery::AdvertisementResult::SUCCESS) {
          // If we can't advertise correctly, shutdown and log the issue.
          server->Shutdown();
          LOG(FATAL) << "Service advertisement failed with advertisement result: "
                     << to_string(result);
        }
      });

  LOG(INFO) << "Server listening on port " << port;
  server->Wait();
}

}  // namespace tvsc::service::echo

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::echo::run_server();

  return 0;
}
