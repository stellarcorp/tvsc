#pragma once

#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/chat/client/chat_streamer.h"
#include "service/chat/client/client.h"

namespace tvsc::service::chat {

template <bool SSL>
void call(uWS::WebSocket<SSL, true, ChatClient> *ws, std::string_view message, uWS::OpCode op) {
  using std::to_string;
  LOG_EVERY_N(INFO, 1000) << "Web socket calling RPC method ChatClient::get_time()";
  ChatClient *client = static_cast<ChatClient *>(ws->getUserData());
  ChatMessage chat_message{};
  chat_message.ParseFromString(std::string{message});
  grpc::Status status = client->post(chat_message);
  if (status.ok()) {
    ws->send("", uWS::OpCode::BINARY);
  } else {
    LOG_EVERY_N(WARNING, 1000) << "RPC failed -- " << status.error_code() << ": "
                               << status.error_message();
    ws->send(std::string{"RPC failed -- "} + to_string(status.error_code()) + ": " +
                 status.error_message(),
             uWS::OpCode::TEXT);
  }
}

template <bool SSL>
void subscribe(uWS::WebSocket<SSL, true, int> *ws) {
  LOG(INFO) << "Web socket subscribing to " << ChatStreamer::TOPIC_NAME;
  ws->subscribe(ChatStreamer::TOPIC_NAME);
}

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> &app) {
  constexpr bool SSL{false};
  app.ws<ChatClient>(base_path + "/post_message",  //
                     uWS::TemplatedApp<SSL>::WebSocketBehavior<ChatClient>{
                         .message = call<SSL>,
                     });
  app.ws<int>(base_path + "/subscribe_to_messages",  //
              uWS::TemplatedApp<SSL>::WebSocketBehavior<int>{
                  .compression = uWS::DISABLED,
                  .maxPayloadLength = 128,
                  .idleTimeout = 8,
                  .maxBackpressure = 0,
                  .closeOnBackpressureLimit = false,
                  .resetIdleTimeoutOnSend = true,
                  .sendPingsAutomatically = false,
                  .open = subscribe<SSL>,
              });
}

}  // namespace tvsc::service::chat
