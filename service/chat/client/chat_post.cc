#include <chrono>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>

#include "discovery/service_resolver.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/chat/client/client.h"
#include "service/chat/common/chat.grpc.pb.h"

DEFINE_string(sender, "me",
              "Name of person sending the message. Defaults to 'me' since it's just a test program "
              "after all.");
DEFINE_string(message, "Hello, world!", "Message to send. Defaults to 'Hello, world!'");

namespace tvsc::service::chat {

grpc::Status send_message(const std::string& message_text, const std::string& sender) {
  ChatClient client{};
  ChatMessage message{};

  message.set_poster(sender);
  message.set_message(message_text);

  DLOG(INFO) << "chat_post.cc::send_message() -- message: " << message.DebugString();
  return client.post(message);
}

}  // namespace tvsc::service::chat

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  tvsc::discovery::register_mdns_grpc_resolver();

  auto status = tvsc::service::chat::send_message(FLAGS_message, FLAGS_sender);
  if (!status.ok()) {
    LOG(WARNING) << "Error. status: " << status.error_message();
    return status.error_code();
  }

  return 0;
}
