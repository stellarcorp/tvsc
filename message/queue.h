#pragma once

#include <array>
#include <cstdint>

#include "message/message.h"
#include "message/processor.h"
#include "message/ring_buffer.h"

namespace tvsc::message {

/**
 * Simple fixed size message queue.
 *
 * This message queue is implemented as a ring buffer, using begin and end pointers (indices) and an
 * array for storage.
 *
 * A fixed number of processors for the messages can be added, but once added, they cannot be
 * removed. The idea is to configure the processors early on and never change them. Currently, we do
 * not see any need to change the processors configuration once it has been set up.
 */
template <typename MessageT, size_t MAX_QUEUE_SIZE, size_t MAX_PROCESSORS>
class Queue final {
 public:
  using MessageType = MessageT;
  using ProcessorType = Processor<MessageType>;

 private:
  RingBuffer<MessageType, MAX_QUEUE_SIZE> messages_{};
  std::array<ProcessorType*, MAX_PROCESSORS> processors_{};

 public:
  [[nodiscard]] bool attach_processor(ProcessorType& p) {
    for (size_t i = 0; i < processors_.size(); ++i) {
      if (processors_[i] == nullptr) {
        processors_[i] = &p;
        return true;
      }
    }
    return false;
  }

  size_t num_processors() const {
    for (size_t i = 0; i < processors_.size(); ++i) {
      if (processors_[i] == nullptr) {
        return i;
      }
    }
    return processors_.size();
  }

  size_t size() const { return messages_.size(); }
  constexpr size_t capacity() const { return messages_.capacity(); }

  [[nodiscard]] bool enqueue(const MessageType& msg) { return messages_.push(msg); }

  bool has_message() const { return !messages_.is_empty(); }

  const auto& peek(size_t index = 0) const { return messages_.peek(index); }

  void process_next_message() {
    if (!messages_.is_empty()) {
      const auto& msg{messages_.peek()};
      for (auto processor : processors_) {
        if (processor == nullptr) {
          // We are out of processors.
          break;
        }
        if (processor->process(msg)) {
          // This processor handled the message, so no other processors need be invoked.
          break;
        }
      }
      // Whether it was handled or not, this message gets dropped from the queue.
      messages_.pop();
    }
  }
};

template <size_t QUEUE_SIZE, size_t NUM_PROCESSORS>
using CanBusMessageQueue = Queue<CanBusMessage, QUEUE_SIZE, NUM_PROCESSORS>;

}  // namespace tvsc::message
