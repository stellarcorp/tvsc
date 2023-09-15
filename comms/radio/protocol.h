#pragma once

#include <cstdint>
#include <string>

namespace tvsc::comms::radio {

enum class Protocol : uint8_t {
  INET,  // Any forwarded "Internet" traffic. Note that this includes INET, INET6, ICMP, and other
         // protocols, not just inet.
  // The values below here all use values that should not conflict with those found in
  // /etc/protocols on most Unix-based systems.
  TVSC_CONTROL = 176,  // 0xb0

  // TODO(james): Rethink whether telemetry should be its own protocol.
  TVSC_TELEMETRY,
};

std::string to_string(tvsc::comms::radio::Protocol protocol);

}  // namespace tvsc::comms::radio
