#pragma once

#include <cstdint>
#include <type_traits>

#include "buffer/buffer.h"
#include "radio/fragment.h"
#include "radio/packet.h"

namespace tvsc::radio {

/**
 * An encoding of a packet into transmittable fragment buffers. Note that any information about the
 * number of fragments needs to be included in the Fragment data itself.
 */
template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
struct EncodedPacket final {
  size_t num_fragments{};
  tvsc::buffer::Buffer<Fragment<MTU>, MAX_FRAGMENTS_PER_PACKET> buffers{};
};

template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
std::string to_string(const EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET>& encoded_packet) {
  using std::to_string;
  std::string result{};
  result.append("num_fragments: ").append(to_string(encoded_packet.num_fragments)).append("\n");
  for (size_t i = 0; i < encoded_packet.num_fragments; ++i) {
    result.append("fragment ")
        .append(to_string(i))
        .append(":\n")
        .append(to_string(encoded_packet.buffers[i]))
        .append("\n");
  }
  return result;
}

/**
 * Encode is a generic operation to translate a single packet structure into transmittable
 * fragments. The identity of each fragment, as well as an indicator for the number of fragments,
 * should be encoded directly into the Fragment data.
 */
template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET,
          size_t PACKET_MAX_PAYLOAD_SIZE = DEFAULT_PACKET_MAX_PAYLOAD_SIZE>
void encode(const PacketT<PACKET_MAX_PAYLOAD_SIZE>& packet,
            EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET>& fragments) {
  static_assert(MAX_FRAGMENTS_PER_PACKET < 128,
                "MAX_FRAGMENTS_PER_PACKET is too large. We only reserve 7 bits for this value (8 "
                "bits including continuation bit).");
  static_assert(std::is_same<uint8_t, std::underlying_type_t<Protocol>>::value,
                "We expect the Protocol to be representable in a single octet. If the underlying "
                "type for that enum is larger than that, this function must be updated.");

  uint8_t fragment_index{0};
  size_t bytes_written{0};
  Fragment<MTU>* current_fragment{nullptr};
  size_t remaining_payload{packet.payload_length()};
  bool have_written_payload_size{false};
  bool have_more_to_encode{true};

  for (fragment_index = 0; fragment_index < MAX_FRAGMENTS_PER_PACKET && have_more_to_encode;
       ++fragment_index) {
    current_fragment = &fragments.buffers[fragment_index];
    bytes_written = 0;

    // These fields are included in every fragment.
    current_fragment->data[bytes_written++] =
        static_cast<std::underlying_type_t<Protocol>>(packet.protocol());
    current_fragment->data[bytes_written++] = packet.sender_id();
    current_fragment->data[bytes_written++] = packet.destination_id();
    current_fragment->data[bytes_written++] =
        static_cast<uint8_t>((packet.sequence_number() >> 8) & 0xff);
    current_fragment->data[bytes_written++] = static_cast<uint8_t>(packet.sequence_number() & 0xff);
    current_fragment->data[bytes_written++] = fragment_index;

    // Payload size only needs to be written in the first fragment.
    if (!have_written_payload_size) {
      size_t payload_size{packet.payload_length()};
      for (uint8_t i = 0; i < Packet::payload_size_bytes_required(); ++i) {
        // Needs to be in network byte order.
        current_fragment->data[bytes_written + Packet::payload_size_bytes_required() - i - 1] =
            payload_size & 0xff;
        payload_size >>= 8;
      }
      bytes_written += Packet::payload_size_bytes_required();
      have_written_payload_size = true;
    }

    if (bytes_written < MTU && remaining_payload > 0) {
      // Now we can add the payload.
      size_t amount_to_write = std::min(remaining_payload, MTU - bytes_written);
      current_fragment->data.write(
          bytes_written, amount_to_write,
          packet.payload().data() + packet.payload_length() - remaining_payload);
      remaining_payload -= amount_to_write;
      bytes_written += amount_to_write;
    }
    current_fragment->length = bytes_written;

    have_more_to_encode = !have_written_payload_size || remaining_payload > 0;
  }

  fragments.num_fragments = fragment_index;

  // Set the continuation bit on all fragments except the last to indicate there are more fragments
  // to come.
  for (uint8_t i = 0; i < fragments.num_fragments - 1; ++i) {
    current_fragment = &fragments.buffers[fragment_index];
    current_fragment->data[5] = current_fragment->data[5] | 0x80;
  }
}

/**
 * Assemble a number of fragments into a single packet. The fragments do *not* need to be ordered
 * correctly in the fragments structure. The assemble function should handle out of order fragments.
 */
template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET,
          size_t PACKET_MAX_PAYLOAD_SIZE = DEFAULT_PACKET_MAX_PAYLOAD_SIZE>
void assemble(const EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET>& fragments,
              PacketT<PACKET_MAX_PAYLOAD_SIZE>& packet) {
  using std::to_string;
  size_t bytes_read{0};
  size_t header_size{0};
  size_t payload_bytes_read{0};
  const Fragment<MTU>* current_fragment{nullptr};

  for (uint8_t fragment_index = 0; fragment_index < fragments.num_fragments; ++fragment_index) {
    current_fragment = &fragments.buffers[fragment_index];
    bytes_read = 0;

    if (fragment_index == 0) {
      // These fields are included in every fragment.
      packet.set_protocol(static_cast<Protocol>(current_fragment->data[bytes_read++]));
      packet.set_sender_id(current_fragment->data[bytes_read++]);
      packet.set_destination_id(current_fragment->data[bytes_read++]);

      packet.set_sequence_number((current_fragment->data[bytes_read] << 8) |
                                 current_fragment->data[bytes_read + 1]);
      bytes_read += 2;

      // Skip over the fragment_index, since the fragments are in the proper order now.
      bytes_read++;

      header_size = bytes_read;

      size_t payload_size{0};
      for (uint8_t i = 0; i < Packet::payload_size_bytes_required(); ++i) {
        payload_size = (payload_size << 8) | current_fragment->data[bytes_read + i];
      }
      bytes_read += Packet::payload_size_bytes_required();
      packet.set_payload_length(payload_size);
    } else {
      bytes_read = header_size;
    }

    // Payload handling.
    const size_t payload_bytes_to_copy{current_fragment->length - bytes_read};
    if (payload_bytes_to_copy > 0) {
      packet.payload().write(payload_bytes_read, payload_bytes_to_copy,
                             current_fragment->data.data() + bytes_read);
      payload_bytes_read += payload_bytes_to_copy;
    }
  }
}

}  // namespace tvsc::radio
