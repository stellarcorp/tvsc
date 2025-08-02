#pragma once

#include <array>
#include <cstdint>

#include "message/handler.h"
#include "message/message.h"

namespace tvsc::message {

/**
 * Simple fixed size message queue.
 *
 * This message queue is implemented as a ring buffer, using begin and end pointers (indices) and an
 * array for storage.
 *
 * A fixed number of handlers for the messages can be added, but once added, they cannot be removed.
 * The idea is to configure the handlers early on and never change them. Currently, we do not see
 * any need to change the handlers configuration once it has been set up.
 */

template <size_t PAYLOAD_SIZE, size_t MAX_QUEUE_SIZE, size_t MAX_HANDLERS>
class Queue final {
 public:
  using HandlerType = Handler<PAYLOAD_SIZE>;
  using MessageType = Message<PAYLOAD_SIZE>;

 private:
  size_t begin_{};
  size_t end_{};
  std::array<MessageType, MAX_QUEUE_SIZE> messages_{};
  std::array<HandlerType*, MAX_HANDLERS> handlers_{};

 public:
  [[nodiscard]] bool attach_handler(HandlerType& h) {
    for (size_t i = 0; i < handlers_.size(); ++i) {
      if (handlers_[i] == nullptr) {
        handlers_[i] = &h;
        return true;
      }
    }
    return false;
  }

  size_t num_handlers() const {
    for (size_t i = 0; i < handlers_.size(); ++i) {
      if (handlers_[i] == nullptr) {
        return i;
      }
    }
    return handlers_.size();
  }

  size_t size() const { return end_ - begin_; }
  size_t capacity() const { return messages_.size(); }

  [[nodiscard]] bool enqueue(const MessageType& msg) {
    if (end_ - begin_ < messages_.size()) {
      messages_[end_ % messages_.size()] = msg;
      ++end_;
      return true;
    } else {
      return false;
    }
  }

  bool has_message() const { return end_ > begin_; }

  const auto& peek(size_t index = 0) const {
    return messages_[(begin_ + index) % messages_.size()];
  }

  void process_next_message() {
    if (end_ - begin_ > 0) {
      const auto& msg{messages_[begin_ % messages_.size()]};
      for (auto handler : handlers_) {
        if (handler == nullptr) {
          // We are out of handlers.
          break;
        }
        if (handler->handle(msg)) {
          // This handler handled the message, so no other handlers need be invoked.
          break;
        }
      }
      // Whether it was handled or not, this message gets dropped from the queue.
      ++begin_;
    }
  }
};

template <size_t QUEUE_SIZE, size_t NUM_HANDLERS>
using CanBusMessageQueue = Queue<CanBusMessage::PAYLOAD_SIZE, QUEUE_SIZE, NUM_HANDLERS>;

}  // namespace tvsc::message
