#include <memory>

#include "RHGenericDriver.h"
#include "RadioHead.h"
#include "glog/logging.h"
#include "gmock/gmock.h"

class MyRadio final : public RHGenericDriver {
 public:
  static constexpr uint8_t MAX_MESSAGE_LENGTH{60};

  bool available() override { return true; }

  bool send(const uint8_t* data, uint8_t len) override { return true; }
  bool recv(uint8_t* buf, uint8_t* len) override { return true; }
  uint8_t maxMessageLength() override { return MAX_MESSAGE_LENGTH; }
};

unsigned long millis() {
  // Stand in for the Arduino function.
  return 0;
}

long random(long, long) {
  // Stand in for the Arduino function.
  return 0;
}

void delay(unsigned long) {
  // Stand in for the Arduino function.
}

class RadioHeadCompileTest : public ::testing::Test {
 public:
  std::unique_ptr<MyRadio> radio{nullptr};

  void SetUp() override { radio.reset(new MyRadio{}); }
};

TEST_F(RadioHeadCompileTest, CanCompileTrivialSendRecv) {
  // Send a message.
  uint8_t data[] = "Hello World!";
  radio->send(data, sizeof(data));

  radio->waitPacketSent();

  // Now wait for a reply
  if (radio->waitAvailableTimeout(500)) {
    uint8_t buf[MyRadio::MAX_MESSAGE_LENGTH];
    uint8_t len{MyRadio::MAX_MESSAGE_LENGTH};

    // Should be a reply message for us now
    if (radio->recv(buf, &len)) {
      LOG(INFO) << "got reply: ";
      LOG(INFO) << (char*)buf;
    } else {
      LOG(INFO) << "recv failed";
    }
  } else {
    LOG(INFO) << "No reply.";
  }
}
