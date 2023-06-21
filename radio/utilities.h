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
#include "radio/transceiver.h"

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

template <size_t MTU>
bool recv(HalfDuplexTransceiver<MTU>& transceiver, Fragment<MTU>& fragment) {
  return transceiver.receive_fragment(fragment, 1000);
}

template <size_t MTU>
bool send(HalfDuplexTransceiver<MTU>& transceiver, const Fragment<MTU>& msg) {
  bool result;
  result = transceiver.transmit_fragment(msg, 250);
  if (result) {
    // Note that we ignore the return value here. The PACKETSENT interrupt is not triggered. That
    // means that we have to wait and assume the packet got transmitted rather than actually
    // knowing.
    // TODO(james): Fix the interrupts in RF69HCW so that we can use this return value.
    const bool wait_succeeded{transceiver.wait_fragment_transmitted(250)};
    if (!wait_succeeded) {
      tvsc::hal::output::println(
          "utilities.h send() -- wait_fragment_transmitted() timed out. Courageously (stupidly) "
          "ignoring.");
    }
  } else {
    tvsc::hal::output::println("transmit_fragment() failed.");
  }

  return result;
}

template <size_t MTU>
void encode_packet(const tvsc_radio_Packet& packet, Fragment<MTU>& fragment) {
  pb_ostream_t ostream =
      pb_ostream_from_buffer(reinterpret_cast<uint8_t*>(fragment.data.data()), fragment.capacity());
  bool status =
      pb_encode(&ostream, nanopb::MessageDescriptor<tvsc_radio_Packet>::fields(), &packet);
  if (!status) {
    tvsc::except<std::runtime_error>("Could not encode packet for message");
  }
  fragment.length = ostream.bytes_written;
}

template <size_t MTU>
void encode_packet(uint32_t protocol, uint32_t sequence_number, uint32_t id,
                   const std::string& message, Fragment<MTU>& fragment) {
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

template <size_t MTU>
bool decode_packet(const Fragment<MTU>& fragment, tvsc_radio_Packet& packet) {
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
