#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "services/echo/client/client.h"

namespace tvsc::service::echo {

template <bool SSL>
void ws_message(uWS::WebSocket<SSL, true, EchoClient> *ws, std::string_view msg, uWS::OpCode op) {
  DLOG(INFO) << "echo::ws_message() -- msg: '" << msg << "', op: " << op;
  EchoClient *client = static_cast<EchoClient *>(ws->getUserData());
  EchoReply reply{};
  grpc::Status status = client->call(std::string{msg}, &reply);
  if (status.ok()) {
    ws->send(reply.msg(), op);
  } else {
    LOG(WARNING) << "RPC failed -- " << status.error_code() << ": " << status.error_message();
    ws->send(std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " + status.error_message(),
             uWS::OpCode::TEXT);
  }
}

uWS::App::WebSocketBehavior<EchoClient> create_web_socket_behavior() {
  constexpr bool SSL{false};
  uWS::TemplatedApp<SSL>::WebSocketBehavior<EchoClient> behavior{
      .message = ws_message<SSL>,
  };

  return behavior;
}

uWS::SSLApp::WebSocketBehavior<EchoClient> create_web_socket_behavior_with_ssl() {
  constexpr bool SSL{true};
  uWS::TemplatedApp<SSL>::WebSocketBehavior<EchoClient> behavior{
      .message = ws_message<SSL>,
  };

  return behavior;
}

}  // namespace tvsc::service::echo
