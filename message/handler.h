#pragma once

#include <cstdint>

#include "message/message.h"

namespace tvsc::message {

template <size_t PAYLOAD_SIZE>
class Handler {
 public:
  virtual ~Handler() {}

  virtual bool handle(const Message<PAYLOAD_SIZE>& msg) = 0;
};

}  // namespace tvsc::message
