#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/datetime/client/client.h"

namespace tvsc::service::datetime {

template <bool SSL>
void call(uWS::WebSocket<SSL, true, DatetimeClient> *ws, std::string_view message, uWS::OpCode op) {
  using std::to_string;
  DatetimeClient *client = static_cast<DatetimeClient *>(ws->getUserData());
  DatetimeReply reply{};
  grpc::Status status = client->call(&reply);
  if (status.ok()) {
    std::string serialized_reply{};
    reply.SerializeToString(&serialized_reply);
    ws->send(serialized_reply, uWS::OpCode::BINARY);
  } else {
    LOG_EVERY_N(WARNING, 1000) << "RPC failed -- " << status.error_code() << ": " << status.error_message();
    ws->send(std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " +
                 status.error_message(),
             uWS::OpCode::TEXT);
  }
}

template <bool SSL>
void stream(uWS::WebSocket<SSL, true, DatetimeClient> *ws, std::string_view message,
            uWS::OpCode op) {
  using std::to_string;
  DatetimeClient *client = static_cast<DatetimeClient *>(ws->getUserData());
  grpc::ClientContext context{};
  auto reader = client->stream(&context);

  DatetimeReply reply{};
  std::string serialized_reply{};
  bool success{true};
  while (success) {
    success = reader->Read(&reply);
    if (success) {
      reply.SerializeToString(&serialized_reply);
      ws->send(serialized_reply, uWS::OpCode::BINARY);
    }
  }
  reader->Finish();
}

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> *app) {
  constexpr bool SSL{false};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = call<SSL>,
          });
  app->ws(base_path + "/get_datetime",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = call<SSL>,
          });
  app->ws(base_path + "/stream_datetime",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = stream<SSL>,
          });
}

void create_web_socket_behaviors_with_ssl(const std::string &base_path,
                                          uWS::TemplatedApp<true> *app) {
  constexpr bool SSL{true};
  app->ws(base_path,  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = call<SSL>,
          });
  app->ws(base_path + "/get_datetime",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = call<SSL>,
          });
  app->ws(base_path + "/stream_datetime",  //
          uWS::TemplatedApp<SSL>::WebSocketBehavior<DatetimeClient>{
              .message = stream<SSL>,
          });
}

}  // namespace tvsc::service::datetime