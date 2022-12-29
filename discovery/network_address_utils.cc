#include "discovery/network_address_utils.h"

#include <exception>
#include <string>

#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

inline void to_hex_string(uint8_t value, char* dest) {
  static constexpr char hex_values[] = "0123456789abcdef";
  *dest = hex_values[value >> 4];
  *(dest + 1) = hex_values[value & 0xf];
}

std::string network_address_to_string(const NetworkAddress& address) {
  if (address.has_ipv4()) {
    return ipv4_address_to_string(address.ipv4());
  } else if (address.has_ipv6()) {
    return ipv6_address_to_string(address.ipv6());
  } else {
    throw std::domain_error("Unimplemented address type in network_address_to_string()");
  }
}

std::string ipv4_address_to_string(const IPv4Address& address) {
  using std::to_string;
  std::string result{};
  // Reserve 16 bytes for result. Maximum form is xxx.xxx.xxx.xxx plus the null terminator.
  result.reserve(16);

  char* dest = result.data();

  to_hex_string(static_cast<uint8_t>((address.address() >> 24) & 0xff), dest);
  dest += 2;
  *dest = '.';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address() >> 16) & 0xff), dest);
  dest += 2;
  *dest = '.';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address() >> 8) & 0xff), dest);
  dest += 2;
  *dest = '.';
  ++dest;
  to_hex_string(static_cast<uint8_t>(address.address() & 0xff), dest);
  dest += 2;

  *dest = '\0';

  return result;
}

std::string ipv6_address_to_string(const IPv6Address& address) {
  std::string result{};
  // Reserve 48 bytes for result. Maximum form is xx: (3 characters) per byte, with a total of 16
  // bytes, minus the final ':' but plus the null terminator.
  result.reserve(48);

  char* dest = result.data();

  // TODO(james): Identify and collapse the longest run of zeros into "::".
  to_hex_string(static_cast<uint8_t>((address.address_0() >> 24) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_0() >> 16) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_0() >> 8) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>(address.address_0() & 0xff), dest);
  dest += 2;

  *dest = ':';
  ++dest;

  to_hex_string(static_cast<uint8_t>((address.address_1() >> 24) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_1() >> 16) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_1() >> 8) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>(address.address_1() & 0xff), dest);
  dest += 2;

  *dest = ':';
  ++dest;

  to_hex_string(static_cast<uint8_t>((address.address_2() >> 24) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_2() >> 16) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_2() >> 8) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>(address.address_2() & 0xff), dest);
  dest += 2;

  *dest = ':';
  ++dest;

  to_hex_string(static_cast<uint8_t>((address.address_3() >> 24) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_3() >> 16) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>((address.address_3() >> 8) & 0xff), dest);
  dest += 2;
  *dest = ':';
  ++dest;
  to_hex_string(static_cast<uint8_t>(address.address_3() & 0xff), dest);
  dest += 2;

  *dest = '\0';

  return result;
}

}  // namespace tvsc::discovery
