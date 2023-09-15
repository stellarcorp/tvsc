#pragma once

#include <cmath>
#include <cstdint>
#include <string>

#include "base/bits.h"
#include "base/except.h"
#include "buffer/buffer.h"
#include "comms/radio/protocol.h"
#include "hash/hash_combine.h"
#include "hash/integer_hash.h"

namespace tvsc::comms::packet {

template <size_t MAX_PAYLOAD_SIZE>
class PacketT final {
 private:
  tvsc::comms::radio::Protocol protocol_{tvsc::comms::radio::Protocol::INET};
  uint8_t sender_id_{0};
  uint8_t destination_id_{0};
  uint16_t sequence_number_{0};
  uint8_t fragment_index_{0};
  bool is_last_fragment_{true};
  size_t payload_length_{0};
  tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE> payload_{};

 public:
  PacketT() = default;
  PacketT(const PacketT& rhs) = default;
  PacketT(PacketT&& rhs) = default;
  PacketT& operator=(const PacketT& rhs) = default;
  PacketT& operator=(PacketT&& rhs) = default;

  PacketT(tvsc::comms::radio::Protocol protocol) : protocol_(protocol) {}

  PacketT(tvsc::comms::radio::Protocol protocol, uint8_t sender_id, uint8_t destination_id)
      : protocol_(protocol), sender_id_(sender_id), destination_id_(destination_id) {}

  PacketT(tvsc::comms::radio::Protocol protocol, uint8_t sender_id, uint8_t destination_id,
          uint16_t sequence_number)
      : protocol_(protocol),
        sender_id_(sender_id),
        destination_id_(destination_id),
        sequence_number_(sequence_number) {}

  PacketT(tvsc::comms::radio::Protocol protocol, uint8_t sender_id, uint8_t destination_id,
          uint16_t sequence_number, size_t payload_length,
          tvsc::buffer::Buffer<uint8_t, MAX_PAYLOAD_SIZE> payload)
      : protocol_(protocol),
        sender_id_(sender_id),
        destination_id_(destination_id),
        sequence_number_(sequence_number),
        payload_length_(payload_length),
        payload_(payload) {}

  static constexpr size_t max_payload_size() { return MAX_PAYLOAD_SIZE; }
  constexpr size_t capacity() const { return max_payload_size(); }

  static constexpr uint8_t payload_size_bits_required() {
    return tvsc::bit_width(MAX_PAYLOAD_SIZE);
  }
  static constexpr uint8_t payload_size_bytes_required() {
    return (payload_size_bits_required() + 7) / 8;
  }

  static constexpr size_t header_size() {
    return sizeof(tvsc::comms::radio::Protocol) + sizeof(uint8_t) + sizeof(uint8_t) +
           sizeof(uint16_t) + sizeof(uint8_t);
  }

  /**
   * Check that the Packet as current configured is valid.
   */
  bool is_valid() const {
    // All we can do is check that the payload length is appropriate for MAX_PAYLOAD_SIZE.
    return payload_length_ <= MAX_PAYLOAD_SIZE;
  }

  tvsc::comms::radio::Protocol protocol() const { return protocol_; }
  void set_protocol(tvsc::comms::radio::Protocol protocol) { protocol_ = protocol; }

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
  void set_payload_length(size_t payload_length) { payload_length_ = payload_length; }

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

  std::size_t header_hash() const {
    std::size_t aggregated{};
    aggregated = tvsc::hash::hash_combine(
        aggregated,
        tvsc::hash::integer_hash(
            static_cast<std::underlying_type_t<tvsc::comms::radio::Protocol>>(protocol_)));
    aggregated = tvsc::hash::hash_combine(aggregated, tvsc::hash::integer_hash(sequence_number_));
    aggregated = tvsc::hash::hash_combine(aggregated, tvsc::hash::integer_hash(sender_id_));
    aggregated = tvsc::hash::hash_combine(aggregated, tvsc::hash::integer_hash(destination_id_));
    return aggregated;
  }

  std::size_t hash() const {
    std::size_t payload_hash{};
    for (size_t i = 0; i < payload_length_; i += 8) {
      uint64_t payload_unit{};
      for (size_t j = 0; j < 8 && i + j < payload_length_; ++j) {
        payload_unit = (payload_unit << 8) | payload_[i + j];
      }
      payload_hash = tvsc::hash::hash_combine(payload_hash, tvsc::hash::integer_hash(payload_unit));
    }
    const std::size_t payload_length_hash{tvsc::hash::integer_hash(payload_length_)};
    const std::size_t header_hash_value{header_hash()};

    return tvsc::hash::hash_combine(payload_hash, payload_length_hash, header_hash_value);
  }
};

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

}  // namespace tvsc::comms::packet
