#pragma once

#include <string>

#include "discovery/service_descriptor.pb.h"

namespace tvsc::discovery {

std::string network_address_to_string(const NetworkAddress& address);
std::string ipv4_address_to_string(const IPv4Address& address);
std::string ipv6_address_to_string(const IPv6Address& address);

}  // namespace tvsc::discovery
