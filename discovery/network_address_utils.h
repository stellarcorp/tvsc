#pragma once

#include <string>
#include <vector>

#include "avahi-common/address.h"
#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

/**
 * Address comparison functions.
 *
 * These functions compare the address part, but ignore the interface name, index, and other fields
 * on the NetworkAddress message.
 */
bool is_same_address(const NetworkAddress& lhs, const NetworkAddress& rhs);
bool is_same_address(const IPv4Address& lhs, const IPv4Address& rhs);
bool is_same_address(const IPv6Address& lhs, const IPv6Address& rhs);

std::string network_address_to_string(const NetworkAddress& address);
std::string ipv4_address_to_string(const IPv4Address& address);
std::string ipv6_address_to_string(const IPv6Address& address);

NetworkAddress avahi_address_to_network_address(AvahiProtocol protocol, const AvahiAddress& address,
                                                int interface);

std::string get_hostname();

/**
 * Get all of the network addresses for the local computer.
 *
 * This includes all addresses for all interfaces in the "up" state, including AF_INET,
 * AF_INET6, and AF_PACKET interfaces.
 *
 * TODO(james): Add support for the AF_PACKET family.
 */
std::vector<NetworkAddress> get_network_addresses();

/**
 * Lookup between the interface name and the interface index.
 *
 * Note that you can get the full set of interfaces (at least interfaces in the "up" state via
 * get_network_addresses().
 */
unsigned int interface_index(const std::string& interface_name);
std::string interface_name(unsigned int interface_index);

}  // namespace tvsc::discovery
