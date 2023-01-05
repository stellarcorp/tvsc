#include "App.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "services/datetime/client/web_socket_rpc_client.h"
#include "services/echo/client/web_socket_rpc_client.h"
#include "services/hello/client/web_socket_rpc_client.h"
#include "services/radio/client/web_socket_rpc_client.h"

DEFINE_int32(port, 50050, "Port to listen on.");

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  uWS::App app{};
  tvsc::service::hello::create_web_socket_behaviors("/service/hello", &app);
  tvsc::service::echo::create_web_socket_behaviors("/service/echo", &app);
  tvsc::service::datetime::create_web_socket_behaviors("/service/datetime", &app);
  tvsc::service::radio::create_web_socket_behaviors("/service/radio", &app);

  app.listen(FLAGS_port,
             [](auto* listen_socket) {
               if (listen_socket) {
                 LOG(INFO) << "Listening on port " << FLAGS_port;
               }
             })
      .run();

  return 0;
}
