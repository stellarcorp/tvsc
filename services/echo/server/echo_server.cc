#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/echo/common/echo.grpc.pb.h"

using echo::Echo;
using echo::EchoReply;
using echo::EchoRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// Logic and data behind the server's behavior.
class EchoServiceImpl final : public Echo::Service {
  Status Echo(ServerContext* context, const EchoRequest* request, EchoReply* reply) override {
    reply->set_msg(request->msg());
    return Status::OK;
  }
};

DEFINE_string(server, "0.0.0.0:50051", "address:port to listen on. Defaults to '0.0.0.0:50051'.");

void RunServer() {
  EchoServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(FLAGS_server, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << FLAGS_server;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  RunServer();

  return 0;
}
