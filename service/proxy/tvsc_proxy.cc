#include <filesystem>

#include "App.h"
#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "http/static_file_server.h"
#include "pubsub/publication_service.h"
#include "pubsub/web_socket_topic.h"
#include "service/datetime/client/datetime_streamer.h"
#include "service/datetime/client/web_socket_rpc_client.h"
#include "service/datetime/common/datetime.pb.h"
#include "service/echo/client/web_socket_rpc_client.h"
#include "service/hello/client/web_socket_rpc_client.h"
#include "service/radio/client/web_socket_rpc_client.h"

DEFINE_int32(port, 50050, "Port to listen on.");
DEFINE_string(doc_root, "service/proxy/doc_root/", "Location of static files.");

constexpr bool SSL{false};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  DLOG(INFO) << "CWD: " << std::filesystem::current_path().native();

  tvsc::discovery::register_mdns_grpc_resolver();

  uWS::TemplatedApp<SSL> app{};
  tvsc::service::hello::create_web_socket_behaviors("/service/hello", app);
  tvsc::service::echo::create_web_socket_behaviors("/service/echo", app);
  tvsc::service::datetime::create_web_socket_behaviors("/service/datetime", app);
  tvsc::service::radio::create_web_socket_behaviors("/service/radio", app);

  tvsc::http::serve_static_files("/static/*", app);
  tvsc::http::serve_homepage("/static/index.html", app);
  tvsc::http::serve_favicon("/static/favicon.ico", app);

  tvsc::pubsub::WebSocketTopic<tvsc::service::datetime::DatetimeReply, SSL, 1> topic{
      tvsc::service::datetime::DatetimeStreamer::TOPIC_NAME, app};
  topic.register_publishing_handler(*uWS::Loop::get());

  tvsc::pubsub::PublicationService<tvsc::service::datetime::DatetimeReply> datetime_publisher{
      topic, std::make_unique<tvsc::service::datetime::DatetimeStreamer>()};
  // TODO(james): Add capability to start this stream on request.
  datetime_publisher.start();

  app.listen(FLAGS_port,
             [](auto* listen_socket) {
               if (listen_socket) {
                 LOG(INFO) << "Listening on port " << FLAGS_port;
               }
             })
      .run();

  return 0;
}
