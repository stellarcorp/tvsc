#include "discovery/network_address_utils.h"

#include <ifaddrs.h>
#include <netinet/in.h>
#include <unistd.h>

#include <exception>
#include <set>
#include <string>

#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

bool is_same_address(const NetworkAddress& lhs, const NetworkAddress& rhs) {
  if (lhs.has_ipv4() and rhs.has_ipv4()) {
    return is_same_address(lhs.ipv4(), rhs.ipv4());
  } else if (lhs.has_ipv6() and rhs.has_ipv6()) {
    return is_same_address(lhs.ipv6(), rhs.ipv6());
  }
  return false;
}

bool is_same_address(const IPv4Address& lhs, const IPv4Address& rhs) {
  return lhs.address() == rhs.address();
}

bool is_same_address(const IPv6Address& lhs, const IPv6Address& rhs) {
  return lhs.address_0() == rhs.address_0() and lhs.address_1() == rhs.address_1() and
         lhs.address_2() == rhs.address_2() and lhs.address_3() == rhs.address_3();
}

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

NetworkAddress avahi_address_to_network_address(AvahiProtocol protocol,
                                                const AvahiAddress& avahi_address) {
  using std::to_string;
  NetworkAddress address{};
  switch (protocol) {
    case AVAHI_PROTO_INET: {
      const AvahiIPv4Address* avahi_ipv4 = &avahi_address.data.ipv4;
      address.mutable_ipv4()->set_address(avahi_ipv4->address);
      break;
    }
    case AVAHI_PROTO_INET6: {
      const AvahiIPv6Address* avahi_ipv6 = &avahi_address.data.ipv6;
      IPv6Address* ipv6 = address.mutable_ipv6();
      for (int i = 0; i < 16; ++i) {
        uint8_t value = avahi_ipv6->address[i];
        if ((i % 4) == 0) {
          ipv6->set_address_0((ipv6->address_0() << 8) + value);
        } else if ((i % 4) == 1) {
          ipv6->set_address_1((ipv6->address_1() << 8) + value);
        } else if ((i % 4) == 2) {
          ipv6->set_address_2((ipv6->address_2() << 8) + value);
        } else if ((i % 4) == 3) {
          ipv6->set_address_3((ipv6->address_3() << 8) + value);
        }
      }
      break;
    }
    case AVAHI_PROTO_UNSPEC:
      break;
    default:
      throw std::domain_error("Unimplemented AvahiProtocol value " + to_string(protocol));
  }

  return address;
}

std::string get_hostname() {
  std::string result{};
  result.reserve(HOST_NAME_MAX + 1);
  gethostname(result.data(), result.capacity());
  return result;
}

std::vector<NetworkAddress> get_network_addresses() {
  std::vector<NetworkAddress> result{};

  std::unique_ptr<struct ifaddrs, void (*)(struct ifaddrs*)> interfaces_list{nullptr, freeifaddrs};
  {
    struct ifaddrs* raw_interfaces_list{nullptr};
    getifaddrs(&raw_interfaces_list);
    interfaces_list.reset(raw_interfaces_list);
  }

  for (struct ifaddrs* current_interface = interfaces_list.get(); current_interface != nullptr;
       current_interface = current_interface->ifa_next) {
    if (current_interface->ifa_addr == nullptr) {
      continue;
    }

    NetworkAddress network_address{};
    network_address.set_interface_name(current_interface->ifa_name);
    if (current_interface->ifa_addr->sa_family == AF_INET) {  // IPv4 family.
      IPv4Address* ip_address{network_address.mutable_ipv4()};
      ip_address->set_address(((struct sockaddr_in*)current_interface->ifa_addr)->sin_addr.s_addr);
    } else if (current_interface->ifa_addr->sa_family == AF_INET6) {  // IPv6 family.
      IPv6Address* ip_address{network_address.mutable_ipv6()};
      uint32_t value{0};
      struct in6_addr* source = &((struct sockaddr_in6*)current_interface->ifa_addr)->sin6_addr;
      for (int i = 0; i < 4; ++i) {
        value = value << 8;
        value += source->s6_addr[i + 0];
      }
      ip_address->set_address_0(value);

      value = 0;
      for (int i = 0; i < 4; ++i) {
        value = value << 8;
        value += source->s6_addr[i + 4];
      }
      ip_address->set_address_1(value);

      value = 0;
      for (int i = 0; i < 4; ++i) {
        value = value << 8;
        value += source->s6_addr[i + 8];
      }
      ip_address->set_address_2(value);

      value = 0;
      for (int i = 0; i < 4; ++i) {
        value = value << 8;
        value += source->s6_addr[i + 12];
      }
      ip_address->set_address_3(value);
    } else if (current_interface->ifa_addr->sa_family == AF_PACKET) {  // Packet sockets.
      // TODO(james): Add support for the AF_PACKET family.
    } else {
      using std::to_string;
      // TODO(james): Any other families we need to support?
      throw std::domain_error("Need to add support for socket family " +
                              to_string(current_interface->ifa_addr->sa_family));
    }

    result.emplace_back(std::move(network_address));
  }

  return result;
}

}  // namespace tvsc::discovery
