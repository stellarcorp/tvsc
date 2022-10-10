#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/echo/common/echo.grpc.pb.h"
#include "services/echo/common/echo_service_location.h"

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

  const std::string bind_addr{get_echo_service_bind_addr()};
  builder.AddListeningPort(bind_addr, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << bind_addr;

  server->Wait();
}

}  // namespace tvsc::service::echo

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::echo::run_server();

  return 0;
}
