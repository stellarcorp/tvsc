#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include "gflags/gflags.h"
#include "services/configuration/service_configuration.h"

DECLARE_string(socket_address);
DECLARE_string(service_configuration);

namespace tvsc::service::configuration {

/**
 * Determine the bind address (hostname and port) for a service. This function is designed to be called with flags from
 * main(): determine_socket_address(SERVICE_NAME, FLAGS_end_point, FLAGS_service_configuration, "ip:port");
 *
 * In particular, if end_point is not an empty string, it is the value that is returned. Otherwise, the value is looked
 * up in the service configuration. Failing both of those approaches, the default value is returned.
 */
std::string determine_socket_address(std::string_view service_name, std::string_view end_point,
                                   const ServiceConfiguration& service_configuration, std::string_view default_value);

/**
 * Convenience function. Calls above using a ServiceConfiguration.
 */
std::string determine_socket_address(std::string_view service_name, std::string_view end_point,
                                   const std::filesystem::path& service_map_location, std::string_view default_value);

/**
 * Convenience function. Calls above using the command line flags and defaulting the bind address to localhost only.
 */
inline std::string determine_socket_address(std::string_view service_name, int default_port) {
  using std::to_string;
  const std::string default_bind{"127.0.0.1:" + to_string(default_port)};
  return determine_socket_address(service_name, FLAGS_socket_address, FLAGS_service_configuration, default_bind);
}

}  // namespace tvsc::service::configuration
