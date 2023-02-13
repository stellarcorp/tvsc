#pragma once

#include <memory>
#include <string>

#include "discovery/service_types.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/chat/common/chat.grpc.pb.h"
#
namespace tvsc::service::chat {

class ChatClient {
 public:
  ChatClient() : ChatClient(tvsc::discovery::service_url<Chat>()) {}

  ChatClient(const std::string& bind_addr)
      : stub_(Chat::NewStub(grpc::CreateChannel(bind_addr, grpc::InsecureChannelCredentials()))) {
    DLOG(INFO) << "ChatClient::ChatClient(std::string)";
  }

  grpc::Status post(const ChatMessage& message) {
    grpc::ClientContext context{};
    EmptyMessage reply{};
    return post(context, message, reply);
  }

  grpc::Status post(grpc::ClientContext& context, const ChatMessage& message, EmptyMessage& reply) {
    return stub_->post_message(&context, message, &reply);
  }

  std::unique_ptr<grpc::ClientReaderInterface<ChatMessage>> subscribe_to_messages(
      grpc::ClientContext& context) {
    const EmptyMessage request{};
    return subscribe_to_messages(context, request);
  }

  std::unique_ptr<grpc::ClientReaderInterface<ChatMessage>> subscribe_to_messages(
      grpc::ClientContext& context, const EmptyMessage& request) {
    return stub_->subscribe_to_messages(&context, request);
  }

  void subscribe_to_messages(grpc::ClientContext& context, const EmptyMessage& request,
                             grpc::ClientReadReactor<ChatMessage>& reactor) {
    stub_->async()->subscribe_to_messages(&context, &request, &reactor);
  }

 private:
  std::unique_ptr<Chat::Stub> stub_;
};

}  // namespace tvsc::service::chat
