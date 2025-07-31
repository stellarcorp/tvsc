#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace tvsc::message {

enum class Type : uint8_t {
  // Lower id numbers have highest priority.
  EMERGENCY = 0,
  PING,      // Used for keep-alive.
  COMMAND,   // Used to issue commands to the satellite or boards in the satellite.
  ANNOUNCE,  // Used to indicate existence.
  TELEMETRY,
};

template <size_t PAYLOAD_SIZE>
class Message final {
 public:
  using Payload = std::array<uint8_t, PAYLOAD_SIZE>;

 private:
  Type type_{};
  size_t size_{};
  Payload payload_{};

 public:
  Message() = default;
  Message(Type type) : type_(type) {}

  Message(const Message& rhs) : type_(rhs.type_), size_(rhs.size_), payload_(rhs.payload_) {}

  Message& operator=(const Message& rhs) {
    type_ = rhs.type_;
    size_ = rhs.size_;
    payload_ = rhs.payload_;
    return *this;
  }

  Type type() const { return type_; }
  void set_type(Type type) { type_ = type; }

  size_t size() const { return size_; }
  void set_size(size_t size) { size_ = size; }

  const Payload& payload() const { return payload_; }
  Payload& payload() { return payload_; }

  size_t append_payload(size_t size, const uint8_t* data) {
    const size_t amount_to_copy{std::min(PAYLOAD_SIZE - size_, size)};
    std::memcpy(reinterpret_cast<void*>(payload_.data() + size_),
                reinterpret_cast<const void*>(data), amount_to_copy);
    size_ += amount_to_copy;
    return amount_to_copy;
  }

  void clear_payload() {
    size_ = 0;
    std::memset(reinterpret_cast<void*>(payload_.data()), 0, PAYLOAD_SIZE);
  }
};

using CanBusMessage = Message<8>;

}  // namespace tvsc::message
