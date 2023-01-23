#include "discovery/network_address_utils.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#include <exception>
#include <set>
#include <string>

#include "discovery/service_descriptor.pb.h"
#include "glog/logging.h"

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
  char result[INET_ADDRSTRLEN + 1];
  struct in_addr inet_address {};
  inet_address.s_addr = address.address();
  const char* return_value = inet_ntop(AF_INET, &inet_address, result, INET_ADDRSTRLEN);
  if (return_value == nullptr) {
    LOG(ERROR) << "inet_ntop() failed. errno: " << strerror(errno);
  }
  return result;
}

std::string ipv6_address_to_string(const IPv6Address& address) {
  char result[INET6_ADDRSTRLEN + 1];
  struct in6_addr inet_address {};
  for (int i = 0; i < 16; ++i) {
    uint32_t value{0};
    if ((i / 4) == 0) {
      value = address.address_0();
    } else if ((i / 4) == 1) {
      value = address.address_1();
    } else if ((i / 4) == 2) {
      value = address.address_2();
    } else if ((i / 4) == 3) {
      value = address.address_3();
    }
    for (int j = 0; j < i % 4; ++j) {
      value = value >> 8;
    }
    inet_address.s6_addr[i] = static_cast<unsigned char>(value & 0xf);
  }
  inet_ntop(AF_INET6, &inet_address, result, INET6_ADDRSTRLEN);
  return result;
}

NetworkAddress avahi_address_to_network_address(AvahiProtocol protocol,
                                                const AvahiAddress& avahi_address, int interface) {
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
  address.set_interface_index(interface);
  address.set_interface_name(interface_name(interface));

  return address;
}

void resolved_server_to_grpc(const ServerDetails& server, grpc_resolved_address* grpc) {
  const NetworkAddress& address{server.address()};

  memset(grpc, 0, sizeof(grpc_resolved_address));
  if (address.has_ipv4()) {
    struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(grpc->addr);
    addr->sin_family = AF_INET;
    addr->sin_port = htons(server.port());
    grpc->len = static_cast<socklen_t>(sizeof(struct sockaddr_in));

    const IPv4Address& ip_address{address.ipv4()};
    addr->sin_addr.s_addr = ip_address.address();
  } else if (address.has_ipv6()) {
    struct sockaddr_in6* addr = reinterpret_cast<struct sockaddr_in6*>(grpc->addr);
    addr->sin6_family = AF_INET6;
    addr->sin6_port = htons(server.port());
    grpc->len = static_cast<socklen_t>(sizeof(struct sockaddr_in6));

    const IPv6Address& ip_address{address.ipv6()};

    uint32_t addr_bytes{0};
    for (int i = 0; i < 16; ++i) {
      if (i % 4 == 0) {
        switch (i / 4) {
          case 0:
            addr_bytes = ip_address.address_0();
            break;
          case 1:
            addr_bytes = ip_address.address_1();
            break;
          case 2:
            addr_bytes = ip_address.address_2();
            break;
          case 3:
            addr_bytes = ip_address.address_3();
            break;
        }
      }
      addr->sin6_addr.s6_addr[i] = addr_bytes & 0xff;
      addr_bytes >>= 8 * (i % 4);
    }
  }
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
    // TODO(james): We can get the index of each interface using if_nameindex(), but that function
    // doesn't seem to have the address for the interface.
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

unsigned int interface_index(const std::string& interface_name) {
  return if_nametoindex(interface_name.c_str());
}

std::string interface_name(unsigned int interface_index) {
  char result[IF_NAMESIZE + 1];
  if_indextoname(interface_index, result);
  return result;
}

}  // namespace tvsc::discovery
