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
  TVSC_CONTROL = 176,  // 0xb0
};

template <size_t MAX_PAYLOAD_SIZE>
class PacketT final {
 private:
  Protocol protocol_{};
  uint8_t sender_id_{};
  uint8_t destination_id_{};
  uint16_t sequence_number_{};
  uint8_t fragment_index_{};
  bool is_last_fragment_{true};
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

  /**
   * The fragment index is only used when assembling a packet from fragments. It does not
   * participate in the identity of the Packet (via the equality or hash functions). It is just for
   * bookkeeping when assembling packets and should not be set at any other time.
   */
  // TODO(james): Remove this field and find a better way.
  uint8_t fragment_index() const { return fragment_index_; }
  void set_fragment_index(uint8_t fragment_index) {
    // We use the MSB as a "continuation bit". If that bit is set, there are more fragments coming.
    // The last fragment in the packet has a zero in this bit.
    if (fragment_index >= 0x80) {
      is_last_fragment_ = false;
      fragment_index -= 0x80;
    }
    fragment_index_ = fragment_index;
  }

  /**
   * The is_last_fragment_flag is only used when assembling a packet from fragments. It does not
   * participate in the identity of the Packet (via the equality or hash functions). It is just for
   * bookkeeping when assembling packets and should not be set at any other time.
   */
  // TODO(james): Remove this field and find a better way.
  bool is_last_fragment() const { return is_last_fragment_; }
  void set_is_last_fragment(bool is_last_fragment) { is_last_fragment_ = is_last_fragment; }

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

  /**
   * Determine if two packets have the same headers (we ignore payload length for this purpose).
   * Usually, this will mean that they are part of the same "session" or "connection" or, at least,
   * that they go together in some fashion.
   */
  bool same_header(const PacketT& rhs) const {
    return protocol_ == rhs.protocol_ &&              //
           sender_id_ == rhs.sender_id_ &&            //
           destination_id_ == rhs.destination_id_ &&  //
           sequence_number_ == rhs.sequence_number_;
  }

 private:
  // Put these hash utilities in a private section, since it's not part of the public API, but keep
  // it close to the hash function(s) that use it.
  // TODO(james): Move these hash utilities to their own class/package.
  static inline uint64_t ror64(uint64_t value, uint8_t bits) {
    return (value >> bits) | (value << (64 - bits));
  }

  template <uint8_t bits>
  static inline uint64_t rotate(uint64_t value) {
    return (value >> bits) | (value << (64 - bits));
  }

  // rrxmrrxmsx_0 from Pelle Evensen
  // http://mostlymangling.blogspot.com/2019/01/better-stronger-mixer-and-test-procedure.html
  static inline uint64_t integer_hash(uint64_t value) {
    value ^= rotate<25>(value) ^ rotate<50>(value);
    value *= 0xA24BAED4963EE407UL;
    value ^= rotate<24>(value) ^ rotate<49>(value);
    value *= 0x9FB21C651E98DF25UL;
    return value ^ (value >> 28);
  }

  static inline uint64_t integer_hash(uint32_t value) {
    uint64_t expanded{value};
    expanded = (expanded << 32) | expanded;
    return integer_hash(expanded);
  }

  static inline uint64_t integer_hash(uint16_t value) {
    uint32_t expanded{value};
    expanded = (expanded << 16) | expanded;
    return integer_hash(expanded);
  }

  static inline uint64_t integer_hash(uint8_t value) {
    uint16_t expanded{value};
    expanded = (expanded << 8) | expanded;
    return integer_hash(expanded);
  }

 public:
  uint64_t header_hash() const {
    uint64_t aggregated{};
    aggregated = (aggregated << 8) | static_cast<std::underlying_type_t<Protocol>>(protocol_);
    aggregated = (aggregated << 16) | sequence_number_;
    aggregated = (aggregated << 8) | sender_id_;
    aggregated = (aggregated << 8) | destination_id_;
    return integer_hash(aggregated);
  }

  uint64_t hash() const {
    uint64_t payload_hash{};
    for (size_t i = 0; i < payload_length_; i += 8) {
      uint64_t payload_unit{};
      for (size_t j = 0; j < 8 && i + j < payload_length_; ++j) {
        payload_unit = (payload_unit << 8) | payload_[i + j];
      }
      payload_hash = payload_hash ^ integer_hash(payload_unit);
    }
    uint64_t payload_length_hash{integer_hash(payload_length_)};
    uint64_t header_hash_value{header_hash()};

    return payload_hash ^ payload_length_hash ^ header_hash_value;
  }
};

std::string to_string(Protocol protocol);

template <size_t MAX_PAYLOAD_SIZE>
std::string to_string(const PacketT<MAX_PAYLOAD_SIZE>& packet) {
  using std::to_string;
  std::string result{};
  result.append("<")
      .append("protocol: ")
      .append(to_string(packet.protocol()))
      .append(", ")
      .append("send: ")
      .append(to_string(packet.sender_id()))
      .append(", ")
      .append("dest: ")
      .append(to_string(packet.destination_id()))
      .append(", ")
      .append("seq#: ")
      .append(to_string(packet.sequence_number()))
      .append(", ")
      .append("frag#: ")
      .append(to_string(packet.fragment_index()))
      .append(", ")
      .append("payload length: ")
      .append(to_string(packet.payload_length()))
      .append(">");
  return result;
}

// By default, we work with packets that have a maximum payload size about 30% smaller than that of
// an ethernet frame.
constexpr size_t DEFAULT_PACKET_MAX_PAYLOAD_SIZE{1024};
using Packet = PacketT<DEFAULT_PACKET_MAX_PAYLOAD_SIZE>;

}  // namespace tvsc::radio
