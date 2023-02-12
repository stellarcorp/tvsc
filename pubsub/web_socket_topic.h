#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "App.h"
#include "pubsub/topic.h"

namespace tvsc::pubsub {

template <typename MessageT, bool SSL, size_t MAX_QUEUE_SIZE = 128>
class WebSocketTopic final : public Topic<MessageT> {
 private:
  std::vector<std::string> message_queue_{};
  std::mutex mu_{};
  uWS::TemplatedApp<SSL>* app_;

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
  WebSocketTopic(std::string_view topic_name, uWS::TemplatedApp<SSL>& app)
      : Topic<MessageT>(topic_name), app_(&app) {}

  void transfer_messages() {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    DLOG_EVERY_N(INFO, 1000) << "WebSocketTopic::transfer_messages() -- now: " << ctime(&now);

    std::lock_guard<std::mutex> l{mu_};
    for (const auto& msg : message_queue_) {
      // Ignore the return value of this call. As far as I can tell, it will return true if it is
      // successful and if there exists a subscriber to the message. So, we can't differentiate
      // between a failure in the process where we would normally throw an exception and a lack of
      // interest where we might perform an optimization.
      app_->publish(this->topic_name_, msg, uWS::OpCode::BINARY, should_compress<MessageT>());
    }
    message_queue_.clear();
  }

  /**
   * All of the web socket message sending and publishing must occur on the thread(s) allocated to
   * the uWS::App instance's Loop. Trying to publish from other threads to the same Loop instance
   * tends to cause deadlock/hanging issues.
   */
  void register_publishing_handler(uWS::Loop& loop) {
    loop.addPreHandler(this, [this](uWS::Loop* /*unused*/) { this->transfer_messages(); });
  }
};

}  // namespace tvsc::pubsub
