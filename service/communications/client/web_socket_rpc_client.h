#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "service/communications/client/client.h"
#include "service/communications/client/rx_streamer.h"
#include "service/communications/common/communications.pb.h"

namespace tvsc::service::communications {

template <bool SSL>
void handle_list_radios(uWS::WebSocket<SSL, true, CommunicationsClient> *ws, std::string_view msg,
                        uWS::OpCode op) {
  LOG(INFO) << "communications::handle_list_radios() -- op: " << op;
  CommunicationsClient *client = static_cast<CommunicationsClient *>(ws->getUserData());
  Radios reply{};
  grpc::Status status = client->list_radios(&reply);
  if (status.ok()) {
    LOG(INFO) << "communications::handle_list_radios() -- sending reply: " << reply.DebugString();
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
void handle_transmit(uWS::WebSocket<SSL, true, CommunicationsClient> *ws,
                     std::string_view request_text, uWS::OpCode op) {
  LOG(INFO) << "communications::handle_transmit() -- request_text: '" << request_text
            << "', op: " << op;
  CommunicationsClient *client = static_cast<CommunicationsClient *>(ws->getUserData());

  Message request{};
  if (op == uWS::OpCode::TEXT) {
    // TODO(james): Hack. Remove this and force the browser to send binary messages for all RPC and
    // stream requests.
    request.set_message(std::string{request_text});
  } else if (op == uWS::OpCode::BINARY) {
    request.ParseFromString(std::string{request_text});
  }

  SuccessResult reply{};
  grpc::Status status = client->transmit(request, &reply);
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
void subscribe(uWS::WebSocket<SSL, true, int> *ws) {
  LOG(INFO) << "Web socket subscribing to " << RxStreamer::TOPIC_NAME;
  ws->subscribe(RxStreamer::TOPIC_NAME);
}

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> &app) {
  constexpr bool SSL{false};
  app.ws(base_path + "/list_radios",  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<CommunicationsClient>{
             .message = handle_list_radios<SSL>,
         });
  app.ws(base_path + "/transmit",  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<CommunicationsClient>{
             .message = handle_transmit<SSL>,
         });

  app.ws<int>(base_path + "/receive",  //
              uWS::TemplatedApp<SSL>::WebSocketBehavior<int>{
                  .compression = uWS::DISABLED,
                  .maxPayloadLength = 128,
                  .idleTimeout = 960,
                  .maxBackpressure = 0,
                  .closeOnBackpressureLimit = false,
                  .resetIdleTimeoutOnSend = true,
                  .sendPingsAutomatically = false,
                  .open = subscribe<SSL>,
              });
}

}  // namespace tvsc::service::communications
