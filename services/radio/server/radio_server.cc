#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/radio/common/radio.grpc.pb.h"
#include "services/radio/common/radio_service_location.h"
#include "soapy_server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace tvsc::service::radio {

class RadioServiceImpl final : public Radio::Service {
  Status radio(ServerContext* context, const RadioRequest* request, RadioReply* reply) override {
    const std::string& msg{request->msg()};
    reply->set_msg(msg);
    LOG(INFO) << "Received msg: '" << msg << "'";
    return Status::OK;
  }
};

void run_server() {
  RadioServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  ServerBuilder builder;

  const std::string bind_addr{get_radio_service_socket_address()};
  builder.AddListeningPort(bind_addr, grpc::InsecureServerCredentials());

  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  LOG(INFO) << "Server listening on " << bind_addr;

  server->Wait();
}

}  // namespace tvsc::service::radio

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::thread soapy_protocol_handler{run_soapy_server};

  tvsc::service::radio::run_server();

  soapy_protocol_handler.join();

  return 0;
}
