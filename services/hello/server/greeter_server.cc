#include <iostream>
#include <memory>
#include <string>

#include "discovery/service_advertiser.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/configuration/service_types.h"
#include "services/hello/common/hello.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace tvsc::service::hello {

class HelloServiceImpl final : public Hello::Service {
  Status say_hello(ServerContext* context, const HelloRequest* request,
                   HelloReply* reply) override {
    const std::string name{request->name()};
    LOG(INFO) << "Request received -- name: '" << name << "'";
    reply->set_msg("Hello, " + name + "!");
    return Status::OK;
  }
};

void run_server() {
  HelloServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;

  int port{0};
  builder.AddListeningPort("dns:///[::]:0", grpc::InsecureServerCredentials(), &port);

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());

  tvsc::discovery::ServiceAdvertiser advertiser{};
  advertiser.advertise_service(
      "TVSC Greeting Service", tvsc::service::configuration::generate_service_type<Hello>(),
      "local", port, [&server](tvsc::discovery::AdvertisementResult result) {
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

}  // namespace tvsc::service::hello

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::hello::run_server();

  return 0;
}
