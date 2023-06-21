#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

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

    if (bytes_written < MTU && remaining_payload > 0) {
      // Now we can add the payload.
      const size_t fragment_payload_size = std::min(
          remaining_payload, MTU - (bytes_written + Packet::payload_size_bytes_required()));

      // To write out the fragment's payload size, we use a shift and mask approach that destroys
      // the data in the variable. Since we need this data later, we make a copy and destroy the
      // copy.
      size_t fragment_payload_size_copy{fragment_payload_size};
      for (uint8_t i = 0; i < Packet::payload_size_bytes_required(); ++i) {
        // Needs to be in network byte order.
        current_fragment->data[bytes_written + Packet::payload_size_bytes_required() - i - 1] =
            fragment_payload_size_copy & 0xff;
        fragment_payload_size_copy >>= 8;
      }

      bytes_written += Packet::payload_size_bytes_required();

      current_fragment->data.write_array(
          bytes_written, fragment_payload_size,
          packet.payload().data() + packet.payload_length() - remaining_payload);

      bytes_written += fragment_payload_size;
      remaining_payload -= fragment_payload_size;
    }

    current_fragment->length = bytes_written;
    have_more_to_encode = remaining_payload > 0;
  }

  fragments.num_fragments = fragment_index;

  // Set the continuation bit on all fragments except the last to indicate there are more fragments
  // to come.
  for (uint8_t i = 0; i < fragments.num_fragments - 1; ++i) {
    current_fragment = &fragments.buffers[i];
    current_fragment->data[5] = current_fragment->data[5] | 0x80;
  }
}

template <size_t MTU, size_t PACKET_MAX_PAYLOAD_SIZE>
bool decode(const Fragment<MTU>& fragment, PacketT<PACKET_MAX_PAYLOAD_SIZE>& packet) {
  // TODO(james): Add assertions on expected invariants.
  using std::to_string;
  size_t bytes_read{0};
  size_t payload_bytes_read{0};

  // Header.
  packet.set_protocol(static_cast<Protocol>(fragment.data[bytes_read++]));
  packet.set_sender_id(fragment.data[bytes_read++]);
  packet.set_destination_id(fragment.data[bytes_read++]);

  packet.set_sequence_number((fragment.data[bytes_read] << 8) | fragment.data[bytes_read + 1]);
  bytes_read += 2;

  packet.set_fragment_index(fragment.data[bytes_read++]);

  // Fragments without a payload won't even have a payload size. In that case, we are done.
  if (bytes_read < fragment.length) {
    size_t payload_size{0};
    for (uint8_t i = 0; i < Packet::payload_size_bytes_required(); ++i) {
      payload_size = (payload_size << 8) | fragment.data[bytes_read + i];
    }

    bytes_read += Packet::payload_size_bytes_required();
    packet.set_payload_length(payload_size);

    // Payload handling.
    if (payload_size > 0) {
      packet.payload().write_array(0, payload_size, fragment.data.data() + bytes_read);
      payload_bytes_read += payload_size;
    }
  }

  return true;
}

/**
 * Assemble a number of fragments into a single packet. The list of fragments (designated by the two
 * iterators) does *not* need to be ordered correctly.
 */
template <size_t PACKET_MAX_PAYLOAD_SIZE, typename IteratorT>
bool assemble(IteratorT fragments_begin, IteratorT fragments_end,
              PacketT<PACKET_MAX_PAYLOAD_SIZE>& packet) {
  bool found_last_fragment{false};
  size_t fragment_index{0};
  while (!found_last_fragment) {
    bool found_fragment_with_index{false};
    for (auto iter = fragments_begin; iter != fragments_end; ++iter) {
      const PacketT<PACKET_MAX_PAYLOAD_SIZE>& fragment{*iter};
      if (fragment.fragment_index() == fragment_index) {
        packet.set_protocol(fragment.protocol());
        packet.set_sender_id(fragment.sender_id());
        packet.set_destination_id(fragment.destination_id());
        packet.set_sequence_number(fragment.sequence_number());

        // Copy payload.
        if (fragment.payload_length() > 0) {
          packet.payload().write_array(packet.payload_length(), fragment.payload_length(),
                                       fragment.payload().data());

          packet.set_payload_length(packet.payload_length() + fragment.payload_length());
        }
        found_fragment_with_index = true;
        if (fragment.is_last_fragment()) {
          found_last_fragment = true;
        }
        break;
      }
    }
    if (found_fragment_with_index) {
      ++fragment_index;
    } else {
      // Don't have all of the fragments to assemble the full packet.
      return false;
    }
  }

  return true;
}

}  // namespace tvsc::radio
