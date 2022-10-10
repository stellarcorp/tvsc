#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/hello/common/hello.grpc.pb.h"
#include "services/hello/common/hello_service_location.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace tvsc::service::hello {

class HelloServiceImpl final : public Hello::Service {
  Status say_hello(ServerContext* context, const HelloRequest* request, HelloReply* reply) override {
    const std::string name{request->name()};
    LOG(INFO) << "Request received -- name: '" << name << "'";
    reply->set_msg("Hello, " + name + "!");
    return Status::OK;
  }
};

void run_server() {
  HelloServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;

  const std::string bind_addr{get_hello_service_bind_addr()};
  builder.AddListeningPort(bind_addr, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << bind_addr;

  server->Wait();
}

}  // namespace tvsc::service::hello

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::service::hello::run_server();

  return 0;
}
