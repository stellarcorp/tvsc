#include <filesystem>

#include "App.h"
#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "http/static_file_server.h"
#include "pubsub/publication_service.h"
#include "pubsub/web_socket_topic.h"
#include "service/chat/client/chat_streamer.h"
#include "service/chat/client/web_socket_rpc_client.h"
#include "service/communications/client/rx_streamer.h"
#include "service/communications/client/web_socket_rpc_client.h"
#include "service/datetime/client/datetime_streamer.h"
#include "service/datetime/client/web_socket_rpc_client.h"
#include "service/datetime/common/datetime.pb.h"
#include "service/echo/client/web_socket_rpc_client.h"
#include "service/hello/client/web_socket_rpc_client.h"

DEFINE_int32(port, 50050, "Port to listen on.");
DEFINE_string(doc_root, "service/proxy/doc_root/", "Location of static files.");

constexpr bool SSL{false};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  DLOG(INFO) << "CWD: " << std::filesystem::current_path().native();

  tvsc::discovery::register_mdns_grpc_resolver();

  uWS::TemplatedApp<SSL> app{};
  // Serve static files from the /static path.
  tvsc::http::serve_static_files("/static/*", app);
  // Special logic to serve the homepage and favicon from "/" and "/favicon.ico".
  tvsc::http::serve_homepage("/static/index.html", app);
  tvsc::http::serve_favicon("/static/favicon.ico", app);

  // Web sockets for each service. These handle methods with vanilla RPC mechanics and can set up
  // subscriptions to streams.
  tvsc::service::chat::create_web_socket_behaviors("/service/chat", app);
  tvsc::service::datetime::create_web_socket_behaviors("/service/datetime", app);
  tvsc::service::echo::create_web_socket_behaviors("/service/echo", app);
  tvsc::service::hello::create_web_socket_behaviors("/service/hello", app);
  tvsc::service::communications::create_web_socket_behaviors("/service/communications", app);

  // Publish the stream of chat messages.
  tvsc::pubsub::WebSocketTopic<tvsc::service::chat::ChatMessage, SSL, 1> chat_topic{
      tvsc::service::chat::ChatStreamer::TOPIC_NAME, app};
  chat_topic.register_publishing_handler(*uWS::Loop::get());

  tvsc::pubsub::PublicationService<tvsc::service::chat::ChatMessage> chat_publisher{
      chat_topic, std::make_unique<tvsc::service::chat::ChatStreamer>()};
  // TODO(james): Add capability to start this stream on request.
  chat_publisher.start();

  // Publish the stream of datetime messages.
  tvsc::pubsub::WebSocketTopic<tvsc::service::datetime::DatetimeReply, SSL, 1> datetime_topic{
      tvsc::service::datetime::DatetimeStreamer::TOPIC_NAME, app};
  datetime_topic.register_publishing_handler(*uWS::Loop::get());

  tvsc::pubsub::PublicationService<tvsc::service::datetime::DatetimeReply> datetime_publisher{
      datetime_topic, std::make_unique<tvsc::service::datetime::DatetimeStreamer>()};
  // TODO(james): Add capability to start this stream on request.
  datetime_publisher.start();

  // Publish the stream of datetime messages.
  tvsc::pubsub::WebSocketTopic<tvsc::service::communications::Message, SSL, 1> radio_rx_topic{
      tvsc::service::communications::RxStreamer::TOPIC_NAME, app};
  radio_rx_topic.register_publishing_handler(*uWS::Loop::get());

  tvsc::pubsub::PublicationService<tvsc::service::communications::Message> radio_rx_publisher{
      radio_rx_topic, std::make_unique<tvsc::service::communications::RxStreamer>()};
  // TODO(james): Add capability to start this stream on request.
  radio_rx_publisher.start();

  app.listen(FLAGS_port,
             [](auto* listen_socket) {
               if (listen_socket) {
                 LOG(INFO) << "Listening on port " << FLAGS_port;
               }
             })
      .run();

  return 0;
}
