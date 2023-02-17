#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <vector>

#include "grpcpp/grpcpp.h"
#include "service/chat/common/chat.grpc.pb.h"
#include "service/chat/common/chat.pb.h"

namespace tvsc::service::chat {

class ChatServiceImpl final : public Chat::Service {
 private:
  std::mutex mu_{};
  std::condition_variable cv_{};

  std::map<grpc::ServerWriter<ChatMessage>*, std::vector<ChatMessage>> writer_queues_{};

 public:
  grpc::Status post_message(grpc::ServerContext* context, const ChatMessage* request,
                            EmptyMessage* reply) override;

  grpc::Status subscribe_to_messages(grpc::ServerContext* context, const EmptyMessage* request,
                                     grpc::ServerWriter<ChatMessage>* writer) override;
};

}  // namespace tvsc::service::chat
