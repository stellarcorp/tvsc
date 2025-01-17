#pragma once

#include <cstdint>
#include <string>

#include "bits/bits.h"
#include "buffer/buffer.h"

namespace tvsc::comms::radio {

/**
 * A fragment is the unit of information that a radio can receive and transmit. A fragment
 * may include all of the information of a single packet, or it may be smaller than a packet.
 * Fragments should not include information from multiple packets.
 */
template <size_t MTU>
class Fragment final {
 public:
  static constexpr uint8_t payload_size_bytes_required() {
    if constexpr (bits::bit_width(MTU - PAYLOAD_OFFSET) < 8) {
      return 1;
    } else if constexpr (bits::bit_width(MTU - PAYLOAD_OFFSET) < 16) {
      return 2;
    } else if constexpr (bits::bit_width(MTU - PAYLOAD_OFFSET) < 24) {
      return 3;
    } else if constexpr (bits::bit_width(MTU - PAYLOAD_OFFSET) < 32) {
      return 4;
    } else {
      // If we have more than a 32-bit int of payload size, we can optimize payload computations for
      // speed and readability rather than trying to squeeze every last byte out of the fragment.
      return 8;
    }
  }

 private:
  static constexpr size_t HEADER_OFFSET{0};
  static constexpr size_t SENDER_ID_OFFSET{HEADER_OFFSET};
  static constexpr size_t SENDER_ID_SIZE{1};
  static constexpr size_t DESTINATION_ID_OFFSET{SENDER_ID_OFFSET + SENDER_ID_SIZE};
  static constexpr size_t DESTINATION_ID_SIZE{1};
  static constexpr size_t FRAGMENT_INDEX_OFFSET{DESTINATION_ID_OFFSET + DESTINATION_ID_SIZE};
  static constexpr size_t FRAGMENT_INDEX_SIZE{1};
  static constexpr size_t SEQUENCE_NUMBER_OFFSET{FRAGMENT_INDEX_OFFSET + FRAGMENT_INDEX_SIZE};
  static constexpr size_t SEQUENCE_NUMBER_SIZE{2};
  static constexpr size_t HEADER_SIZE{SENDER_ID_SIZE + DESTINATION_ID_SIZE + FRAGMENT_INDEX_SIZE +
                                      SEQUENCE_NUMBER_SIZE};

  static constexpr size_t PAYLOAD_OFFSET{HEADER_OFFSET + HEADER_SIZE};
  static constexpr size_t PAYLOAD_SIZE_OFFSET{PAYLOAD_OFFSET};

  static_assert(PAYLOAD_OFFSET == SEQUENCE_NUMBER_OFFSET + SEQUENCE_NUMBER_SIZE,
                "Error in Fragment field offsets or sizes.");
  static_assert(PAYLOAD_OFFSET + 2 <= MTU,
                "MTU is too small to support any payload. Need at least space for the header plus "
                "one size byte plus one payload byte.");

 public:
  // TODO(james): Make these two fields private.
  static constexpr size_t PAYLOAD_DATA_OFFSET{PAYLOAD_SIZE_OFFSET + payload_size_bytes_required()};
  tvsc::buffer::Buffer<uint8_t, MTU> data{};

  static constexpr size_t mtu() { return MTU; }
  static constexpr size_t header_size() { return HEADER_SIZE; }
  static constexpr size_t max_payload_size() {
    return mtu() - header_size() - payload_size_bytes_required();
  }

  /**
   * Validate the fragment as much as possible. Currently, this just means checking the length is
   * less than or equal to the MTU.
   */
  bool is_valid() const { return total_length() <= MTU; }

  uint8_t sender_id() const { return data[SENDER_ID_OFFSET]; }
  void set_sender_id(uint8_t id) { data[SENDER_ID_OFFSET] = id; }

  uint8_t destination_id() const { return data[DESTINATION_ID_OFFSET]; }
  void set_destination_id(uint8_t id) { data[DESTINATION_ID_OFFSET] = id; }

  uint16_t sequence_number() const {
    uint16_t result{data[SEQUENCE_NUMBER_OFFSET]};
    result = (result << 8) | data[SEQUENCE_NUMBER_OFFSET + 1];
    return result;
  }

  void set_sequence_number(uint16_t sequence_number) {
    data[SEQUENCE_NUMBER_OFFSET] = static_cast<uint8_t>((sequence_number >> 8) & 0xff);
    data[SEQUENCE_NUMBER_OFFSET + 1] = static_cast<uint8_t>(sequence_number & 0xff);
  }

  uint8_t fragment_index() const { return data[FRAGMENT_INDEX_OFFSET] & 0x7f; }
  void set_fragment_index(uint8_t index) {
    data[FRAGMENT_INDEX_OFFSET] = index | (data[FRAGMENT_INDEX_OFFSET] & 0x80);
  }

  bool is_continued() const { return (data[FRAGMENT_INDEX_OFFSET] & 0x80) != 0; }
  void set_continuation_flag() { data[FRAGMENT_INDEX_OFFSET] = data[FRAGMENT_INDEX_OFFSET] | 0x80; }
  void clear_continuation_flag() {
    data[FRAGMENT_INDEX_OFFSET] = data[FRAGMENT_INDEX_OFFSET] & 0x7f;
  }

  size_t payload_size() const {
    size_t result{};
    for (int i = 0; i < payload_size_bytes_required(); ++i) {
      result <<= 8;
      result |= data[PAYLOAD_SIZE_OFFSET + i];
    }
    return result;
  }

  void set_payload_size(size_t size) {
    for (int i = payload_size_bytes_required() - 1; i >= 0; --i) {
      data[PAYLOAD_SIZE_OFFSET + i] = static_cast<uint8_t>(size & 0xff);
      size >>= 8;
    }
  }

  uint8_t* payload_start() { return data.data() + PAYLOAD_DATA_OFFSET; }
  const uint8_t* payload_start() const { return data.data() + PAYLOAD_DATA_OFFSET; }

  size_t total_length() const {
    return HEADER_SIZE + payload_size_bytes_required() + payload_size();
  }

  void clear() { data.clear(); }
};

template <size_t MTU>
std::string to_string(const Fragment<MTU>& fragment) {
  using std::to_string;
  std::string result{};

  result.append("Raw fragment:\n");
  result.append("length: ").append(to_string(fragment.total_length())).append("\n");
  result.append("data:\n").append(to_string(fragment.data)).append("\n");
  return result;
}

}  // namespace tvsc::comms::radio
