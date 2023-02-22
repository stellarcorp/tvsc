#include <string>
#include <string_view>

#include "App.h"
#include "glog/logging.h"
#include "radio/radio.pb.h"
#include "service/communications/client/client.h"

namespace tvsc::service::communications {

template <bool SSL>
void handle_list_radios(uWS::WebSocket<SSL, true, CommunicationsClient> *ws, std::string_view msg,
                        uWS::OpCode op) {
  DLOG(INFO) << "communications::handle_list_radios() -- msg: '" << msg << "', op: " << op;
  CommunicationsClient *client = static_cast<CommunicationsClient *>(ws->getUserData());
  tvsc::radio::Radios reply{};
  grpc::Status status = client->list_radios(&reply);
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

void create_web_socket_behaviors(const std::string &base_path, uWS::TemplatedApp<false> &app) {
  constexpr bool SSL{false};
  app.ws(base_path + "/list_radios",  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<CommunicationsClient>{
             .message = handle_list_radios<SSL>,
         });
}

void create_web_socket_behaviors_with_ssl(const std::string &base_path,
                                          uWS::TemplatedApp<true> &app) {
  constexpr bool SSL{true};
  app.ws(base_path + "/list_radios",  //
         uWS::TemplatedApp<SSL>::WebSocketBehavior<CommunicationsClient>{
             .message = handle_list_radios<SSL>,
         });
}

}  // namespace tvsc::service::communications
