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
    ws->send(std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " +
                 status.error_message(),
             uWS::OpCode::TEXT);
  }
}

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> *app) {
  constexpr bool SSL{false};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<RadioClient>{
              .message = ws_message<SSL>,
          });
  app->ws(base_path + "/echo",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<RadioClient>{
              .message = ws_message<SSL>,
          });
}

void create_web_socket_behaviors_with_ssl(const std::string &base_path,
                                          uWS::TemplatedApp<true> *app) {
  constexpr bool SSL{true};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<RadioClient>{
              .message = ws_message<SSL>,
          });
  app->ws(base_path + "/echo",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<RadioClient>{
              .message = ws_message<SSL>,
          });
}

}  // namespace tvsc::service::radio
