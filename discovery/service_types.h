#pragma once

#include <string>

namespace tvsc::discovery {

/**
 * Scheme to use for gRPC URLs to discover advertised services.
 */
constexpr char MDNS_SCHEME[]{"mdns"};

/**
 * Generated gRPC classes contain a static method called service_full_name(). See the
 * Hello::service_full_name() as an example. This method translates that service name into an mDNS
 * service type. The templated form below is likely the one you want.
 */
std::string generate_service_type_from_name(const std::string& grpc_service_full_name);

template <class GrpcServiceT>
std::string generate_service_type() {
  return generate_service_type_from_name(GrpcServiceT::service_full_name());
}

/**
 * Construct a URL for the service given as a template parameter. This URL will have the form of
 * mdns://<authority>/_grpc_service_full_name._tcp where <authority> is the empty string, meaning to
 * use the local/default authority.
 */
template <class GrpcServiceT>
std::string service_url() {
  return std::string{MDNS_SCHEME} + ":///" + generate_service_type<GrpcServiceT>();
}

}  // namespace tvsc::discovery
