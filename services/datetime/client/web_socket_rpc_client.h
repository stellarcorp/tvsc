#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "services/datetime/client/client.h"

namespace tvsc::service::datetime {

template <bool SSL>
void ws_message(uWS::WebSocket<SSL, true, DatetimeClient> *ws, std::string_view message,
                uWS::OpCode op) {
  using std::to_string;
  DatetimeClient *client = static_cast<DatetimeClient *>(ws->getUserData());
  DatetimeReply reply{};
  grpc::Status status = client->call(&reply);
  if (status.ok()) {
    ws->send(to_string(reply.datetime()), uWS::OpCode::TEXT);
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
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = ws_message<SSL>,
          });
  app->ws(base_path + "/get_datetime",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = ws_message<SSL>,
          });
}

void create_web_socket_behaviors_with_ssl(const std::string &base_path,
                                          uWS::TemplatedApp<true> *app) {
  constexpr bool SSL{true};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = ws_message<SSL>,
          });
  app->ws(base_path + "/get_datetime",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = ws_message<SSL>,
          });
}

}  // namespace tvsc::service::datetime
