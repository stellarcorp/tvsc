#include <iostream>
#include <string>

#include "base/initializer.h"
#include "discovery/service_resolver.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/chat/client/client.h"
#include "service/chat/common/chat.grpc.pb.h"

namespace tvsc::service::chat {

void listen() {
  DLOG(INFO) << "listen()";
  ChatClient client{};
  ChatMessage message{};
  grpc::ClientContext context{};

  DLOG(INFO) << "listen() -- subscribing.";
  auto reader = client.subscribe_to_messages(context);
  while (reader->Read(&message)) {
    DLOG(INFO) << "listen() -- printing messages.";
    std::cout << "Received message:\n" << message.DebugString() << "\n";
  }
  DLOG(INFO) << "listen() -- exiting.";
}

}  // namespace tvsc::service::chat

int main(int argc, char** argv) {
  tvsc::initialize(&argc, &argv);
  tvsc::discovery::register_mdns_grpc_resolver();

  tvsc::service::chat::listen();

  return 0;
}
