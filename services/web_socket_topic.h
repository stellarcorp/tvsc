#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <string_view>

#include "App.h"
#include "services/pub_sub_service.h"

namespace tvsc::services {

template <typename MessageT, size_t MAX_QUEUE_SIZE = 128>
class WebSocketTopic final : public Topic<MessageT> {
 private:
  std::vector<std::string> message_queue_{};
  std::mutex mu_{};

  void publish(const MessageT& message) override {
    std::string serialized{};
    serialized.reserve(message.ByteSizeLong());
    message.SerializeToString(&serialized);
    std::lock_guard<std::mutex> l{mu_};
    if (message_queue_.size() >= MAX_QUEUE_SIZE) {
      message_queue_.erase(message_queue_.begin(),
                           message_queue_.begin() + (message_queue_.size() - MAX_QUEUE_SIZE + 1));
    }
    message_queue_.emplace_back(std::move(serialized));
  }

 public:
  WebSocketTopic(std::string_view topic_name) : Topic<MessageT>(topic_name) {}

  template <bool SSL>
  void transfer_messages(uWS::TemplatedApp<SSL>& app) {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    LOG(INFO) << "WebSocketTopic::transfer_messages() -- now: " << ctime(&now);

    std::lock_guard<std::mutex> l{mu_};
    for (const auto& msg : message_queue_) {
      // Ignore the return value of this call. As far as I can tell, it will return true if it is
      // successful and if there exists a subscriber to the message. So, we can't differentiate
      // between a failure in the process where we would normally throw an exception and a lack of
      // interest where we might perform an optimization.
      app.publish(this->topic_name_, msg, uWS::OpCode::BINARY, should_compress<MessageT>());
    }
    message_queue_.clear();
  }
};

template <typename MessageT>
std::unique_ptr<Topic<MessageT>> create_web_socket_topic(std::string_view topic_name) {
  return std::make_unique<WebSocketTopic<MessageT>>(topic_name);
}

}  // namespace tvsc::services
