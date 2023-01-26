#pragma once

#include <string>

namespace tvsc::discovery {

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

template <class GrpcServiceT>
std::string default_bind_address() {
  return "mdns:///" + generate_service_type<GrpcServiceT>();
}

}  // namespace tvsc::discovery
