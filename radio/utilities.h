/**
 * Collection of utility functions for the various sample apps in this package. Not meant for
 * general purpose use.
 */
#pragma once

#include <cstdint>
#include <string>

#include "hal/output/output.h"
#include "hal/time/time.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "radio/fragment.h"
#include "radio/packet.pb.h"
#include "radio/radio.pb.h"
#include "radio/rf69hcw.h"
#include "random/random.h"

namespace tvsc::radio {

inline void print_id(const tvsc_radio_RadioIdentification& id) {
  tvsc::hal::output::print("{");
  tvsc::hal::output::print(id.expanded_id);
  tvsc::hal::output::print(", ");
  tvsc::hal::output::print(id.id);
  tvsc::hal::output::print(", ");
  tvsc::hal::output::print(id.name);
  tvsc::hal::output::println("}");
}

inline bool recv(RF69HCW& rf69, Fragment<RF69HCW::max_mtu()>& fragment) {
  return rf69.receive_fragment(fragment, 1000);
}

inline bool send(RF69HCW& rf69, const Fragment<RF69HCW::max_mtu()>& msg) {
  bool result;
  result = rf69.transmit_fragment(msg, 250);
  if (result) {
    result = rf69.wait_fragment_transmitted(250);
  }

  return result;
}

inline void encode_packet(const tvsc_radio_Packet& packet, Fragment<RF69HCW::max_mtu()>& fragment) {
  pb_ostream_t ostream =
      pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(fragment.data.data()), fragment.capacity());
  bool status =
      pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not encode packet for message");
  }
  fragment.length = ostream.bytes_written;
}

inline void encode_packet(uint32_t protocol, uint32_t sequence_number, uint32_t id,
                          const std::string& message, Fragment<RF69HCW::max_mtu()>& fragment) {
  tvsc_radio_Packet packet{};
  packet.protocol = protocol;
  packet.sequence_number = sequence_number;
  packet.sender = id;
  packet.payload.size = std::min(
      message.length(),
      static_cast<std::string::size_type>(
          fragment.capacity() - /* HACK -- This code should not be used in production. */ 10));
  std::strncpy(reinterpret_cast<char*>(packet.payload.bytes), message.data(), packet.payload.size);

  encode_packet(packet, fragment);
}

inline bool decode_packet(const Fragment<RF69HCW::max_mtu()>& fragment, tvsc_radio_Packet& packet) {
  pb_istream_t istream = pb_istream_from_buffer(
      reinterpret_cast<const uint8_t*>(fragment.data.data()), fragment.length);

  bool status =
      pb_decode(&istream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    tvsc::hal::output::println("Could not decode packet");
    return false;
  }

  return true;
}

}  // namespace tvsc::radio
