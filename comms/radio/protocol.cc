#include "comms/radio/protocol.h"

#include <string>

namespace tvsc::comms::radio {

std::string to_string(Protocol protocol) {
  std::string result{};
  switch (protocol) {
    case Protocol::INET:
      result.append("INET");
      break;
    case Protocol::TVSC_CONTROL:
      result.append("TVSC_CONTROL");
      break;
    case Protocol::TVSC_TELEMETRY:
      result.append("TVSC_TELEMETRY");
      break;
  }
  return result;
}

}  // namespace tvsc::comms::radio
