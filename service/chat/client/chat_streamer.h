#pragma once

#include <string>

#include "grpcpp/support/client_callback.h"
#include "pubsub/streamer.h"
#include "service/chat/common/chat.grpc.pb.h"
#include "service/chat/common/chat.pb.h"

namespace tvsc::service::chat {

/**
 * Streaming publisher of the Chat::subscribe_to_messages() method.
 */
class ChatStreamer final : public tvsc::pubsub::GrpcStreamer<Chat, EmptyMessage, ChatMessage> {
 protected:
  void call_rpc_method(Chat::StubInterface::async_interface& async_stub,
                       grpc::ClientContext& context, const EmptyMessage& request,
                       grpc::ClientReadReactor<ChatMessage>& reactor) override {
    async_stub.subscribe_to_messages(&context, &request, &reactor);
  }

 public:
  static constexpr const char TOPIC_NAME[] = "tvsc.service.chat.Chat.subscribe_to_messages";

  ChatStreamer() = default;

  ChatStreamer(const std::string& bind_addr) : GrpcStreamer(bind_addr) {}
};

}  // namespace tvsc::service::chat
