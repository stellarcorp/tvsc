#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "service/echo/client/client.h"

namespace tvsc::service::echo {

template <bool SSL>
void ws_message(uWS::WebSocket<SSL, true, EchoClient> *ws, std::string_view request_text,
                uWS::OpCode op) {
  EchoClient *client = static_cast<EchoClient *>(ws->getUserData());

  EchoRequest request{};
  if (op == uWS::OpCode::TEXT) {
    // TODO(james): Hack. Remove this and force the browser to send binary messages for all RPC and
    // stream requests.
    request.set_msg(std::string{request_text});
  } else if (op == uWS::OpCode::BINARY) {
    request.ParseFromString(std::string{request_text});
  }

  EchoReply reply{};
  grpc::Status status = client->call(request, &reply);
  if (status.ok()) {
    std::string serialized_reply{};
    reply.SerializeToString(&serialized_reply);
    ws->send(serialized_reply, uWS::OpCode::BINARY);
  } else {
    LOG(WARNING) << "RPC failed -- " << status.error_code() << ": " << status.error_message();
    ws->send(std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " +
                 status.error_message(),
             uWS::OpCode::TEXT);
  }
}

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> *app) {
  constexpr bool SSL{false};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<EchoClient>{
              .message = ws_message<SSL>,
          });
  app->ws(base_path + "/echo",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<EchoClient>{
              .message = ws_message<SSL>,
          });
}

void create_web_socket_behaviors_with_ssl(const std::string &base_path,
                                          uWS::TemplatedApp<true> *app) {
  constexpr bool SSL{true};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<EchoClient>{
              .message = ws_message<SSL>,
          });
  app->ws(base_path + "/echo",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<EchoClient>{
              .message = ws_message<SSL>,
          });
}

}  // namespace tvsc::service::echo