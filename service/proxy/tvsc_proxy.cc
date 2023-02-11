#include <filesystem>

#include "App.h"
#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "http/static_file_server.h"
#include "service/datetime/client/web_socket_rpc_client.h"
#include "service/datetime/client/web_socket_streaming_client.h"
#include "service/echo/client/web_socket_rpc_client.h"
#include "service/hello/client/web_socket_rpc_client.h"
#include "service/radio/client/web_socket_rpc_client.h"

DEFINE_int32(port, 50050, "Port to listen on.");
DEFINE_string(doc_root, "service/proxy/doc_root/", "Location of static files.");

constexpr bool SSL{false};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  LOG(INFO) << "CWD: " << std::filesystem::current_path().native();

  tvsc::discovery::register_mdns_grpc_resolver();

  uWS::TemplatedApp<SSL> app{};
  tvsc::service::hello::create_web_socket_behaviors("/service/hello", &app);
  tvsc::service::echo::create_web_socket_behaviors("/service/echo", &app);
  tvsc::service::datetime::create_web_socket_behaviors("/service/datetime", app);
  tvsc::service::radio::create_web_socket_behaviors("/service/radio", &app);

  tvsc::http::serve_static_files("/static/*", &app);
  tvsc::http::serve_homepage("/static/index.html", &app);

  tvsc::service::datetime::StreamingDatetimeClient<SSL> datetime_streamer{app};
  datetime_streamer.register_publishing_handler(*uWS::Loop::get());

  // TODO(james): Add capability to start this stream on request.
  datetime_streamer.start();

  app.listen(FLAGS_port,
             [](auto* listen_socket) {
               if (listen_socket) {
                 LOG(INFO) << "Listening on port " << FLAGS_port;
               }
             })
      .run();

  return 0;
}
