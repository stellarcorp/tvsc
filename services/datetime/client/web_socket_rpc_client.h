#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/datetime/client/client.h"

namespace tvsc::service::datetime {

template <bool SSL>
void ws_message(uWS::WebSocket<SSL, true, DatetimeClient> *ws, std::string_view message, uWS::OpCode op) {
  using std::to_string;
  DatetimeClient *client = static_cast<DatetimeClient *>(ws->getUserData());
  DatetimeReply reply{};
  grpc::Status status = client->call(&reply);
  if (status.ok()) {
    ws->send(to_string(reply.datetime()), uWS::OpCode::TEXT);
  } else {
    LOG(WARNING) << "RPC failed -- " << status.error_code() << ": " << status.error_message();
    ws->send(std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " + status.error_message(),
             uWS::OpCode::TEXT);
  }
}

uWS::App::WebSocketBehavior<DatetimeClient> create_web_socket_behavior() {
  constexpr bool SSL{false};
  uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient> behavior{
      .message = ws_message<SSL>,
  };

  return behavior;
}

uWS::SSLApp::WebSocketBehavior<DatetimeClient> create_web_socket_behavior_with_ssl() {
  constexpr bool SSL{true};
  uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient> behavior{
      .message = ws_message<SSL>,
  };

  return behavior;
}

}  // namespace tvsc::service::datetime
