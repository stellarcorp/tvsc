#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "grpcpp/grpcpp.h"
#include "service/chat/common/chat.grpc.pb.h"
#include "service/chat/common/chat.pb.h"
#include "service/utility/service_runner.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

namespace tvsc::service::chat {

class ChatServiceImpl final : public Chat::Service {
 private:
  std::mutex mu_{};
  std::condition_variable cv_{};

  std::map<grpc::ServerWriter<ChatMessage>*, std::vector<ChatMessage>> writer_queues_{};

 public:
  Status post_message(ServerContext* context, const ChatMessage* request,
                      EmptyMessage* reply) override {
    DLOG(INFO) << "ChatServerImpl::post_message()";
    {
      DLOG(INFO) << "ChatServerImpl::post_message() -- writer_queues_.size(): "
                 << writer_queues_.size();
      std::lock_guard<std::mutex> l(mu_);
      for (auto& entries : writer_queues_) {
        // TODO(james): Make this more efficient. This approach just copies the message to the queue
        // for every writer. Better would be to share a single instance of the message across all of
        // the writers.
        DLOG(INFO) << "ChatServerImpl::post_message() -- posting to writer queue";
        entries.second.push_back(*request);
      }
    }
    DLOG(INFO) << "ChatServerImpl::post_message() -- notifying writers";
    cv_.notify_all();
    return Status::OK;
  }

  Status subscribe_to_messages(ServerContext* context, const EmptyMessage* request,
                               grpc::ServerWriter<ChatMessage>* writer) override {
    using namespace std::literals::chrono_literals;
    DLOG(INFO) << "ChatServerImpl::subscribe_to_messages()";
    std::unique_lock<std::mutex> l(mu_);
    writer_queues_.emplace(writer, std::vector<ChatMessage>{});

    while (!context->IsCancelled()) {
      if (cv_.wait_for(l, 20ms, [context] { return context->IsCancelled(); })) {
        DLOG(INFO) << "ChatServerImpl::subscribe_to_messages() -- context->IsCancelled()";
        break;
      }

      auto& queue{writer_queues_.at(writer)};
      for (const auto& msg : queue) {
        DLOG(INFO) << "ChatServerImpl::subscribe_to_messages() -- writing message.";
        writer->Write(msg);
      }
      queue.clear();
    }
    DLOG(INFO) << "ChatServerImpl::subscribe_to_messages() -- context->IsCancelled(): "
               << (context->IsCancelled() ? "true" : "false");

    writer_queues_.erase(writer);

    DLOG(INFO) << "ChatServerImpl::subscribe_to_messages() -- exiting.";
    // Client-side cancelling of the stream is expected, so we return OK instead of CANCELLED.
    return Status::OK;
  }
};

}  // namespace tvsc::service::chat

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  using namespace tvsc::service::chat;
  tvsc::service::utility::run_single_service<Chat, ChatServiceImpl>("TVSC Chat Service");

  return 0;
}