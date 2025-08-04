#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "base/enums.h"

namespace tvsc::message {

enum class Type : uint8_t {
  // Type is used as the identifier in the Message structure below. Lower id numbers have highest
  // priority to match CAN bus design.
  EMERGENCY = 0,
  PING,      // Used for keep-alive.
  COMMAND,   // Used to issue commands to the satellite or boards in the satellite.
  ANNOUNCE,  // Used to indicate existence.
  TELEMETRY,
};

enum class Subsystem : uint8_t {
  LED = 1,
  MAGNETORQUER = 2,
};

template <size_t MTU>
class Message final {
 public:
  using Payload = std::array<uint8_t, MTU>;

  static constexpr size_t mtu() { return MTU; }

 private:
  uint32_t identifier_{};
  size_t size_{};
  Payload payload_{};

 public:
  Message() = default;
  Message(Type type) : identifier_(cast_to_underlying_type(type)) {}

  Message(const Message& rhs)
      : identifier_(rhs.identifier_), size_(rhs.size_), payload_(rhs.payload_) {}

  Message& operator=(const Message& rhs) {
    identifier_ = rhs.identifier_;
    size_ = rhs.size_;
    payload_ = rhs.payload_;
    return *this;
  }

  bool operator==(const Message& rhs) const {
    return identifier_ == rhs.identifier_ && size_ == rhs.size_ && payload_ == rhs.payload_;
  }

  const uint32_t& identifier() const { return identifier_; }
  uint32_t& identifier() { return identifier_; }

  Type retrieve_type() const {
    auto integral_type{static_cast<std::underlying_type_t<Type>>(identifier_)};
    return static_cast<Type>(integral_type);
  }

  void set_type(Type type) { identifier_ = cast_to_underlying_type(type); }

  size_t size() const { return size_; }
  void set_size(size_t size) { size_ = size; }

  const Payload& payload() const { return payload_; }
  Payload& payload() { return payload_; }

  size_t append_payload(size_t size, const uint8_t* data) {
    const size_t amount_to_copy{std::min(MTU - size_, size)};
    std::memcpy(reinterpret_cast<void*>(payload_.data() + size_),
                reinterpret_cast<const void*>(data), amount_to_copy);
    size_ += amount_to_copy;
    return amount_to_copy;
  }

  void clear_payload() {
    size_ = 0;
    std::memset(reinterpret_cast<void*>(payload_.data()), 0, MTU);
  }
};

using CanBusMessage = Message<8>;

}  // namespace tvsc::message
