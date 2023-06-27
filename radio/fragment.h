#pragma once

#include <cstdint>
#include <string>

#include "buffer/buffer.h"

namespace tvsc::radio {

/**
 * A fragment is the unit of information that a radio can receive and transmit. A fragment
 * may include all of the information of a single packet, or it may be smaller than a packet.
 * Fragments should not include information from multiple packets.
 */
template <size_t MTU>
struct Fragment final {
  size_t length{};
  tvsc::buffer::Buffer<uint8_t, MTU> data{};

  constexpr size_t capacity() const { return MTU; }

  /**
   * Validate the fragment as much as possible. Currently, this just means checking the length is
   * less than or equal to the MTU.
   */
  bool is_valid() const { return length <= MTU; }
};

template <size_t MTU>
std::string to_string(const Fragment<MTU>& fragment) {
  using std::to_string;
  std::string result{};
  result.append("length: ").append(to_string(fragment.length)).append("\n");
  result.append("data:\n").append(to_string(fragment.data)).append("\n");
  return result;
}

}  // namespace tvsc::radio
