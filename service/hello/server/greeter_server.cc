#include <iostream>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/hello/common/hello.grpc.pb.h"
#include "service/utility/service_runner.h"

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

}  // namespace tvsc::service::hello

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::hello;
  tvsc::service::utility::run_single_service<Hello, HelloServiceImpl>("TVSC Greeter Service");

  return 0;
}
