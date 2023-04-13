#include "radio/packet.h"

#include <string>

namespace tvsc::radio {

std::string to_string(Protocol protocol) {
  std::string result{};
  switch (protocol) {
    case Protocol::INET:
      result.append("INET");
      break;
    case Protocol::TVSC_CONTROL:
      result.append("TVSC_CONTROL");
      break;
  }
  return result;
}

}  // namespace tvsc::radio
