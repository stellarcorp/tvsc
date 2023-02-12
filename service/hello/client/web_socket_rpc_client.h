#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "service/hello/client/client.h"

namespace tvsc::service::hello {
inline bool hasBrokenCompression(std::string_view userAgent) {
  size_t posStart = userAgent.find(" Version/15.");
  if (posStart == std::string_view::npos) return false;
  posStart += 12;

  size_t posEnd = userAgent.find(' ', posStart);
  if (posEnd == std::string_view::npos) return false;

  unsigned int minorVersion = 0;
  auto result =
      std::from_chars(userAgent.data() + posStart, userAgent.data() + posEnd, minorVersion);
  if (result.ec != std::errc()) return false;
  if (result.ptr != userAgent.data() + posEnd) return false;  // do not accept trailing chars
  if (minorVersion > 3) return false;                         // we target just Safari 15.0 - 15.3

  if (userAgent.find(" Safari/", posEnd) == std::string_view::npos) return false;

  return true;
}

template <bool SSL>
void ws_upgrade(uWS::HttpResponse<SSL> *res, uWS::HttpRequest *req,
                struct us_socket_context_t *webSocketContext) {
  LOG(INFO) << "hello::ws_upgrade()";

  /* Default handler upgrades to WebSocket */
  std::string_view secWebSocketKey = req->getHeader("sec-websocket-key");
  std::string_view secWebSocketProtocol = req->getHeader("sec-websocket-protocol");
  std::string_view secWebSocketExtensions = req->getHeader("sec-websocket-extensions");

  /* Safari 15 hack */
  if (hasBrokenCompression(req->getHeader("user-agent"))) {
    secWebSocketExtensions = "";
  }

  res->template upgrade<HelloClient>({}, secWebSocketKey, secWebSocketProtocol,
                                     secWebSocketExtensions, webSocketContext);
}

template <bool SSL>
void ws_open(uWS::WebSocket<SSL, true, HelloClient> *ws) {
  LOG(INFO) << "hello::ws_open()";
}

template <bool SSL>
void ws_message(uWS::WebSocket<SSL, true, HelloClient> *ws, std::string_view name, uWS::OpCode op) {
  LOG(INFO) << "hello::ws_message() -- name: '" << name << "', op: " << op;
  HelloClient *client = static_cast<HelloClient *>(ws->getUserData());
  HelloReply reply{};
  grpc::Status status = client->call(std::string{name}, &reply);
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

template <bool SSL>
void ws_drain(uWS::WebSocket<SSL, true, HelloClient> *ws) {
  LOG(INFO) << "hello::ws_drain()";
}

template <bool SSL>
void ws_ping(uWS::WebSocket<SSL, true, HelloClient> *ws, std::string_view message) {
  LOG(INFO) << "hello::ws_ping() -- message: " << message;
  ws->send(message);
}

template <bool SSL>
void ws_pong(uWS::WebSocket<SSL, true, HelloClient> *ws, std::string_view message) {
  LOG(INFO) << "hello::ws_pong() -- message: " << message;
}

template <bool SSL>
void ws_close(uWS::WebSocket<SSL, true, HelloClient> *ws, int, std::string_view message) {
  LOG(INFO) << "hello::ws_close() -- message: " << message;
}

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> &app) {
  constexpr bool SSL{false};
  app.ws(base_path,  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<HelloClient>{
             .upgrade = ws_upgrade<SSL>,
             .open = ws_open<SSL>,
             .message = ws_message<SSL>,
             .drain = ws_drain<SSL>,
             .ping = ws_ping<SSL>,
             .pong = ws_pong<SSL>,
             .close = ws_close<SSL>,
         });
  app.ws(base_path + "/say_hello",  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<HelloClient>{
             .upgrade = ws_upgrade<SSL>,
             .open = ws_open<SSL>,
             .message = ws_message<SSL>,
             .drain = ws_drain<SSL>,
             .ping = ws_ping<SSL>,
             .pong = ws_pong<SSL>,
             .close = ws_close<SSL>,
         });
}

void create_web_socket_behaviors_with_ssl(const std::string &base_path,
                                          uWS::TemplatedApp<true> &app) {
  constexpr bool SSL{true};
  app.ws(base_path,  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<HelloClient>{
             .upgrade = ws_upgrade<SSL>,
             .open = ws_open<SSL>,
             .message = ws_message<SSL>,
             .drain = ws_drain<SSL>,
             .ping = ws_ping<SSL>,
             .pong = ws_pong<SSL>,
             .close = ws_close<SSL>,
         });
  app.ws(base_path + "/say_hello",  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<HelloClient>{
             .upgrade = ws_upgrade<SSL>,
             .open = ws_open<SSL>,
             .message = ws_message<SSL>,
             .drain = ws_drain<SSL>,
             .ping = ws_ping<SSL>,
             .pong = ws_pong<SSL>,
             .close = ws_close<SSL>,
         });
}

}  // namespace tvsc::service::hello
