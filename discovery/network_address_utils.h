#pragma once

#include <string>
#include <vector>

#include "avahi-common/address.h"
#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

bool is_same_address(const NetworkAddress& lhs, const NetworkAddress& rhs);
bool is_same_address(const IPv4Address& lhs, const IPv4Address& rhs);
bool is_same_address(const IPv6Address& lhs, const IPv6Address& rhs);

std::string network_address_to_string(const NetworkAddress& address);
std::string ipv4_address_to_string(const IPv4Address& address);
std::string ipv6_address_to_string(const IPv6Address& address);

NetworkAddress avahi_address_to_network_address(AvahiProtocol protocol,
                                                const AvahiAddress& address);

std::string get_hostname();

/**
 * Get the network addresses for the local computer.
 */
std::vector<NetworkAddress> get_network_addresses();

}  // namespace tvsc::discovery
