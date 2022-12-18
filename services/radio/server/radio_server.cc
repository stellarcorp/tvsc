#include <csignal>
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "services/radio/common/radio.grpc.pb.h"
#include "services/radio/common/radio_service_location.h"
#include "services/radio/server/soapy.h"
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

void run_grpc_server() {
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

// Use a global variable for the Soapy management instance. Signal handlers must be function pointers, and this seems to
// be the easiest way to bind a value (the soapy instance) such that it is accessible from a function pointer. Note that
// the Soapy instance still has automatic storage duration tied to the main() function.
static tvsc::services::radio::server::Soapy* soapy_global{nullptr};
void shutdown_server(int signum) { soapy_global->shutdown_server(); }

}  // namespace tvsc::service::radio

std::string& getModuleLoading(void);

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::services::radio::server::Soapy soapy{};

  LOG(INFO) << "Soapy modules:";
  for (const auto& module : soapy.modules()) {
    LOG(INFO) << module;
  }

  LOG(INFO) << "Soapy devices:";
  for (const auto& device : soapy.devices()) {
    LOG(INFO) << device;
  }

  LOG_IF(WARNING, !soapy.contains_module("libdummy_radio.so")) << "'dummy_radio' module not found";
  LOG_IF(WARNING, !soapy.has_device("dummy_receiver")) << "'dummy_receiver' device not found";

  tvsc::service::radio::soapy_global = &soapy;
  signal(SIGINT, tvsc::service::radio::shutdown_server);

  soapy.start_server();
  std::thread grpc_protocol_handler{tvsc::service::radio::run_grpc_server};

  int soapy_server_result = soapy.wait_on_server();
  LOG(INFO) << "Soapy server result: " << soapy_server_result;
  return 0;
}
