#pragma once

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "buffer/buffer.h"
#include "hal/output/output.h"
#include "radio/fragment.h"
#include "radio/packet.h"

namespace tvsc::radio {

/**
 * An encoding of a packet into transmittable fragment buffers. Note that any information about the
 * number of fragments needs to be included in the Fragment data itself.
 */
template <size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
struct EncodedPacket final {
  size_t num_fragments{0};
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
 *
 * Returns true if packet could be encoded into fragments; false, otherwise.
 */
template <typename PacketT, size_t MTU, size_t MAX_FRAGMENTS_PER_PACKET>
bool encode(const PacketT& packet, EncodedPacket<MTU, MAX_FRAGMENTS_PER_PACKET>& fragments) {
  static_assert(MAX_FRAGMENTS_PER_PACKET < 128,
                "MAX_FRAGMENTS_PER_PACKET is too large. We only reserve 7 bits for this value (8 "
                "bits including continuation bit).");
  static_assert(std::is_same<uint8_t, std::underlying_type_t<Protocol>>::value,
                "We expect the Protocol to be representable in a single octet. If the underlying "
                "type for that enum is larger than that, this function must be updated.");

  if (!packet.is_valid()) {
    return false;
  }

  uint8_t fragment_index{0};
  Fragment<MTU>* current_fragment{nullptr};
  size_t remaining_payload{packet.payload_length()};
  bool have_more_to_encode{true};

  for (fragment_index = 0; fragment_index < MAX_FRAGMENTS_PER_PACKET && have_more_to_encode;
       ++fragment_index) {
    current_fragment = &fragments.buffers[fragment_index];

    // These fields are included in every fragment.
    current_fragment->set_protocol(packet.protocol());
    current_fragment->set_sender_id(packet.sender_id());
    current_fragment->set_destination_id(packet.destination_id());
    current_fragment->set_sequence_number(packet.sequence_number());
    current_fragment->set_fragment_index(fragment_index);

    if (remaining_payload > 0) {
      // Now we can add the payload.
      const size_t fragment_payload_size =
          std::min(remaining_payload, Fragment<MTU>::max_payload_size());

      current_fragment->set_payload_size(fragment_payload_size);

      current_fragment->data.write_array(
          Fragment<MTU>::PAYLOAD_DATA_OFFSET, fragment_payload_size,
          packet.payload().data() + packet.payload_length() - remaining_payload);

      remaining_payload -= fragment_payload_size;
    } else {
      current_fragment->set_payload_size(0);
    }

    have_more_to_encode = remaining_payload > 0;
  }

  fragments.num_fragments = fragment_index;
  if (have_more_to_encode) {
    return false;
  }

  // Set the continuation bit on all fragments except the last to indicate there are more fragments
  // to come.
  for (uint8_t i = 0; i < fragments.num_fragments - 1; ++i) {
    current_fragment = &fragments.buffers[i];
    current_fragment->set_continuation_flag();
  }
  fragments.buffers[fragments.num_fragments - 1].clear_continuation_flag();

  return true;
}

/**
 * Decode a fragment into a Packet.
 *
 * Returns true if it was possible to decode the fragment; false, otherwise.
 */
template <size_t MTU, size_t PACKET_MAX_PAYLOAD_SIZE>
bool decode(const Fragment<MTU>& fragment, PacketT<PACKET_MAX_PAYLOAD_SIZE>& packet) {
  static_assert(Fragment<MTU>::max_payload_size() <= PACKET_MAX_PAYLOAD_SIZE,
                "Packet type not large enough to hold Fragment type. Need Packet type with larger "
                "PACKET_MAX_PAYLOAD_SIZE or need Fragment type with smaller MTU.");

  // TODO(james): Add assertions on expected invariants.
  using std::to_string;

  tvsc::hal::output::println("decode() 1");
  if (!fragment.is_valid()) {
    return false;
  }

  // Header.
  tvsc::hal::output::println("decode() 1");
  packet.set_protocol(fragment.protocol());
  packet.set_sender_id(fragment.sender_id());
  packet.set_destination_id(fragment.destination_id());

  tvsc::hal::output::println("decode() 2");
  packet.set_sequence_number(fragment.sequence_number());

  tvsc::hal::output::println("decode() 3");
  packet.set_fragment_index(fragment.fragment_index());
  if (fragment.is_continued()) {
    packet.set_is_last_fragment(false);
  } else {
    packet.set_is_last_fragment(true);
  }

  tvsc::hal::output::println("decode() 4");
  tvsc::hal::output::print("decode() --  fragment: ");
  tvsc::hal::output::println(to_string(fragment));

  const size_t payload_size{fragment.payload_size()};
  if (payload_size > 0) {
    tvsc::hal::output::println("decode() 5");
    if (payload_size > packet.capacity()) {
      tvsc::hal::output::print(
          "decode() -- payload_size greater than packet capacity. payload_size: ");
      tvsc::hal::output::println(payload_size);
    }

    tvsc::hal::output::println("decode() 6");
    packet.set_payload_length(payload_size);

    tvsc::hal::output::println("decode() 7");
    // Payload handling.
    if (payload_size > 0) {
      tvsc::hal::output::println("decode() 8");
      tvsc::hal::output::print("payload_size: ");
      tvsc::hal::output::println(payload_size);
      packet.payload().write_array(0, payload_size, fragment.payload_start());
      tvsc::hal::output::println("decode() 9");
    }
  }

  tvsc::hal::output::println("decode() 10");
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
