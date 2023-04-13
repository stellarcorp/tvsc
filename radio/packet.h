#pragma once

#include <cmath>
#include <cstdint>
#include <string>

#if __has_cpp_attribute(__cpp_lib_int_pow2)
#include <bit>
#endif

#include "base/except.h"
#include "buffer/buffer.h"

namespace tvsc::radio {

namespace internal {

#if __has_cpp_attribute(__cpp_lib_int_pow2)
inline constexpr size_t number_of_bits(size_t x) noexcept { return std::bit_width(x); }
#else
inline constexpr size_t number_of_bits(size_t x) noexcept {
  return x < 2 ? x : 1 + number_of_bits(x >> 1);
}
#endif

inline constexpr uint8_t bit_width(size_t x) { return static_cast<uint8_t>(number_of_bits(x)); }

}  // namespace internal

enum class Protocol : uint8_t {
  INET,  // Any forwarded "Internet" traffic. Note that this includes INET, INET6, ICMP, and other
         // protocols, not just inet.
  // The values below here all use values that should not conflict with those found in
  // /etc/protocols on most Unix-based systems.
  TVSC_CONTROL = 176,
};

template <size_t MAX_PAYLOAD_SIZE>
class PacketT final {
 private:
  Protocol protocol_{};
  uint8_t sender_id_{};
  uint8_t destination_id_{};
  uint16_t sequence_number_{};
  size_t payload_length_{};
  tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE> payload_{};

 public:
  PacketT() = default;
  PacketT(const PacketT& rhs) = default;
  PacketT(PacketT&& rhs) = default;
  PacketT& operator=(const PacketT& rhs) = default;
  PacketT& operator=(PacketT&& rhs) = default;

  PacketT(Protocol protocol) : protocol_(protocol) {}

  PacketT(Protocol protocol, uint8_t sender_id, uint8_t destination_id)
      : protocol_(protocol), sender_id_(sender_id), destination_id_(destination_id) {}

  PacketT(Protocol protocol, uint8_t sender_id, uint8_t destination_id, uint16_t sequence_number)
      : protocol_(protocol),
        sender_id_(sender_id),
        destination_id_(destination_id),
        sequence_number_(sequence_number) {}

  PacketT(Protocol protocol, uint8_t sender_id, uint8_t destination_id, uint16_t sequence_number,
          size_t payload_length, tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE> payload)
      : protocol_(protocol),
        sender_id_(sender_id),
        destination_id_(destination_id),
        sequence_number_(sequence_number),
        payload_length_(payload_length),
        payload_(payload) {}

  static constexpr size_t max_payload_size() { return MAX_PAYLOAD_SIZE; }
  static constexpr uint8_t payload_size_bits_required() {
    return internal::bit_width(MAX_PAYLOAD_SIZE);
  }
  static constexpr uint8_t payload_size_bytes_required() {
    return (payload_size_bits_required() + 7) / 8;
  }

  Protocol protocol() const { return protocol_; }
  void set_protocol(Protocol protocol) { protocol_ = protocol; }

  uint8_t sender_id() const { return sender_id_; }
  void set_sender_id(uint8_t sender_id) { sender_id_ = sender_id; }

  uint8_t destination_id() const { return destination_id_; }
  void set_destination_id(uint8_t destination_id) { destination_id_ = destination_id; }

  uint16_t sequence_number() const { return sequence_number_; }
  void set_sequence_number(uint16_t sequence_number) { sequence_number_ = sequence_number; }

  size_t payload_length() const { return payload_length_; }
  void set_payload_length(size_t payload_length) {
    if (payload_length > MAX_PAYLOAD_SIZE) {
      using std::to_string;
      except<std::domain_error>(
          "Payload is larger than MAX_PAYLOAD_SIZE. payload_length: " + to_string(payload_length) +
          " (MAX_PAYLOAD_SIZE: " + to_string(MAX_PAYLOAD_SIZE) + ")");
    }
    payload_length_ = payload_length;
  }

  const tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE>& payload() const { return payload_; }
  tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE>& payload() { return payload_; }
  void set_payload(const tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE>& payload) {
    payload_ = payload;
  }

  bool operator==(const PacketT& rhs) const {
    return protocol_ == rhs.protocol_ &&                //
           sender_id_ == rhs.sender_id_ &&              //
           destination_id_ == rhs.destination_id_ &&    //
           sequence_number_ == rhs.sequence_number_ &&  //
           payload_length_ == rhs.payload_length_ &&    //
           payload_.is_equal(rhs.payload_, payload_length_);
  }
};

std::string to_string(Protocol protocol);

template <size_t MAX_PAYLOAD_SIZE>
std::string to_string(const PacketT<MAX_PAYLOAD_SIZE>& packet) {
  using std::to_string;
  std::string result{};
  result.append("<")
      .append(to_string(packet.protocol()))
      .append(", ")
      .append(to_string(packet.sender_id()))
      .append(", ")
      .append(to_string(packet.destination_id()))
      .append(", ")
      .append(to_string(packet.sequence_number()))
      .append(", ")
      .append(to_string(packet.payload_length()))
      .append(">");
  return result;
}

// By default, we work with packets that have a maximum payload size about 30% smaller than that of
// an ethernet frame.
constexpr size_t DEFAULT_PACKET_MAX_PAYLOAD_SIZE{1024};
using Packet = PacketT<DEFAULT_PACKET_MAX_PAYLOAD_SIZE>;

}  // namespace tvsc::radio
