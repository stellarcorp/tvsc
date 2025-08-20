#include "message/queue.h"

#include "gtest/gtest.h"
#include "message/message.h"
#include "message/processor.h"

namespace tvsc::message {

static constexpr size_t DEFAULT_PAYLOAD_SIZE{8};
static constexpr size_t DEFAULT_QUEUE_SIZE{2};
static constexpr size_t DEFAULT_NUM_PROCESSORS{3};

using DefaultMessageType = Message<DEFAULT_PAYLOAD_SIZE>;
using DefaultQueueType = Queue<DefaultMessageType, DEFAULT_QUEUE_SIZE, DEFAULT_NUM_PROCESSORS>;

DefaultMessageType last_message_handled{};
DefaultMessageType last_message_not_handled{};

class AlwaysHandles final : public Processor<DefaultMessageType> {
 private:
  bool was_called_{false};

 public:
  bool process(const DefaultMessageType& msg) override {
    last_message_handled = msg;
    was_called_ = true;
    return true;
  }

  bool was_called() const { return was_called_; }
};

class NeverHandles final : public Processor<DefaultMessageType> {
 private:
  bool was_called_{false};

 public:
  bool process(const DefaultMessageType& msg) override {
    last_message_not_handled = msg;
    was_called_ = true;
    return false;
  }

  bool was_called() const { return was_called_; }
};

TEST(QueueTest, NewQueueIsEmpty) {
  DefaultQueueType queue{};

  EXPECT_FALSE(queue.has_message());
  EXPECT_EQ(queue.size(), 0);
}

TEST(QueueTest, NewQueueHasNoProcessors) {
  DefaultQueueType queue{};

  EXPECT_EQ(queue.num_processors(), 0);
}

TEST(QueueTest, QueueHasExpectedCapacity) {
  DefaultQueueType queue{};

  EXPECT_EQ(queue.capacity(), DEFAULT_QUEUE_SIZE);
}

TEST(QueueTest, CanEnqueueMessage) {
  DefaultQueueType queue{};
  DefaultMessageType initial{Type::PING};
  EXPECT_TRUE(queue.enqueue(initial));

  EXPECT_TRUE(queue.has_message());
  EXPECT_EQ(queue.size(), 1);

  const auto& peeked{queue.peek()};
  EXPECT_EQ(peeked.retrieve_type(), Type::PING);
}

TEST(QueueTest, CanEnqueueMultipleMessages) {
  static constexpr size_t NUM_MESSAGES{DEFAULT_QUEUE_SIZE + 2};

  DefaultQueueType queue{};

  for (size_t i = 0; i < NUM_MESSAGES; ++i) {
    DefaultMessageType msg{Type::PING};
    msg.set_size(i);
    if (i < DEFAULT_QUEUE_SIZE) {
      EXPECT_TRUE(queue.enqueue(msg));
    } else {
      EXPECT_FALSE(queue.enqueue(msg));
      break;
    }
  }

  EXPECT_TRUE(queue.has_message());
  EXPECT_EQ(queue.size(), DEFAULT_QUEUE_SIZE);
}

TEST(QueueTest, CanHandleMessage) {
  DefaultQueueType queue{};
  DefaultMessageType initial{Type::PING};
  EXPECT_TRUE(queue.enqueue(initial));

  EXPECT_TRUE(queue.has_message());
  EXPECT_EQ(queue.size(), 1);

  AlwaysHandles handler{};
  EXPECT_TRUE(queue.attach_processor(handler));

  queue.process_next_message();

  EXPECT_EQ(last_message_handled, initial);

  EXPECT_FALSE(queue.has_message());
  EXPECT_EQ(queue.size(), 0);

  EXPECT_TRUE(handler.was_called());
}

TEST(QueueTest, CanFindCorrectProcessor) {
  DefaultQueueType queue{};
  DefaultMessageType initial{Type::PING};
  EXPECT_TRUE(queue.enqueue(initial));

  EXPECT_TRUE(queue.has_message());
  EXPECT_EQ(queue.size(), 1);

  NeverHandles wrong_handler_1{};
  AlwaysHandles right_handler{};
  NeverHandles wrong_handler_2{};
  EXPECT_TRUE(queue.attach_processor(wrong_handler_1));
  EXPECT_TRUE(queue.attach_processor(right_handler));
  EXPECT_TRUE(queue.attach_processor(wrong_handler_2));

  queue.process_next_message();

  EXPECT_EQ(last_message_not_handled, initial);
  EXPECT_EQ(last_message_handled, initial);

  EXPECT_TRUE(wrong_handler_1.was_called());
  EXPECT_TRUE(right_handler.was_called());
  EXPECT_FALSE(wrong_handler_2.was_called());
}

TEST(QueueTest, RejectsAddingTooManyProcessors) {
  DefaultQueueType queue{};
  std::array<AlwaysHandles, DEFAULT_NUM_PROCESSORS + 1> processors{};
  for (size_t i = 0; i < processors.size(); ++i) {
    auto& processor{processors[i]};
    if (i < DEFAULT_NUM_PROCESSORS) {
      EXPECT_TRUE(queue.attach_processor(processor));
    } else {
      EXPECT_FALSE(queue.attach_processor(processor));
    }
  }
}

}  // namespace tvsc::message
