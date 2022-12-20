#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "services/radio/client/client.h"

namespace tvsc::service::radio {

template <bool SSL>
void ws_message(uWS::WebSocket<SSL, true, RadioClient> *ws, std::string_view msg, uWS::OpCode op) {
  DLOG(INFO) << "radio::ws_message() -- msg: '" << msg << "', op: " << op;
  RadioClient *client = static_cast<RadioClient *>(ws->getUserData());
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

uWS::App::WebSocketBehavior<RadioClient> create_web_socket_behavior() {
  constexpr bool SSL{false};
  uWS::TemplatedApp<SSL>::WebSocketBehavior<RadioClient> behavior{
      .message = ws_message<SSL>,
  };

  return behavior;
}

uWS::SSLApp::WebSocketBehavior<RadioClient> create_web_socket_behavior_with_ssl() {
  constexpr bool SSL{true};
  uWS::TemplatedApp<SSL>::WebSocketBehavior<RadioClient> behavior{
      .message = ws_message<SSL>,
  };

  return behavior;
}

}  // namespace tvsc::service::radio
