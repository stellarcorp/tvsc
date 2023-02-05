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
#include "radio/soapy.h"
#include "radio/soapy_server.h"
#include "service/radio/common/radio.grpc.pb.h"

namespace tvsc::service::radio {

class RadioServiceImpl final : public RadioService::Service {
 private:
  tvsc::radio::Soapy* soapy_;

 public:
  RadioServiceImpl(tvsc::radio::Soapy& soapy) : soapy_(&soapy) {}

  grpc::Status list_radios(grpc::ServerContext* context, const RadioListRequest* request,
                           Radios* reply) override {
    LOG(INFO) << "list_radios() called.";
    for (const auto& device : soapy_->devices()) {
      Radio* radio = reply->add_radios();
      for (const auto& entry : device) {
        if (entry.first == "label") {
          radio->set_name(entry.second);
        }
        KeyValue* key_value = radio->add_keys_values();
        key_value->set_key(entry.first);
        key_value->set_value(entry.second);
      }
    }
    return grpc::Status::OK;
  }
};

void run_grpc_server(RadioService::Service* service) {
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

// Use a global variable for the SoapyServer management instance. Signal handlers must be function
// pointers, and this seems to be the easiest way to bind a value (the soapy instance) such that it
// is accessible from a function pointer. Note that the Soapy instance still has automatic storage
// duration tied to the main() function.
static tvsc::radio::SoapyServer* soapy_global{nullptr};
void shutdown_server(int signum) { soapy_global->shutdown(); }

}  // namespace tvsc::service::radio

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::radio::Soapy soapy{};

  tvsc::radio::SoapyServer soapy_server{soapy};
  tvsc::service::radio::soapy_global = &soapy_server;
  signal(SIGINT, tvsc::service::radio::shutdown_server);

  soapy_server.start();

  tvsc::service::radio::RadioServiceImpl grpc_service{soapy};
  std::thread grpc_protocol_handler{tvsc::service::radio::run_grpc_server, &grpc_service};

  int soapy_server_result = soapy_server.wait();
  LOG(INFO) << "Soapy server result: " << soapy_server_result;
  return 0;
}
