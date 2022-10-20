#include "App.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "services/configuration/ports.h"
#include "services/datetime/client/web_socket_rpc_client.h"
#include "services/echo/client/web_socket_rpc_client.h"
#include "services/hello/client/web_socket_rpc_client.h"

DEFINE_int32(port, static_cast<int>(tvsc::service::configuration::DefaultPort::PROXY_SERVICE), "Port to listen on.");

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  uWS::App app{};

  app.ws<tvsc::service::hello::HelloClient>("/service/hello", tvsc::service::hello::create_web_socket_behavior());
  app.ws<tvsc::service::echo::EchoClient>("/service/echo", tvsc::service::echo::create_web_socket_behavior());
  app.ws<tvsc::service::datetime::DatetimeClient>("/service/datetime",
                                                  tvsc::service::datetime::create_web_socket_behavior());

  app.listen(FLAGS_port,
             [](auto* listen_socket) {
               if (listen_socket) {
                 LOG(INFO) << "Listening on port " << FLAGS_port;
               }
             })
      .run();

  return 0;
}
