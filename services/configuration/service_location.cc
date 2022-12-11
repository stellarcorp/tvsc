#include "services/configuration/service_location.h"

#include <filesystem>
#include <string>
#include <string_view>

#include "gflags/gflags.h"
#include "services/configuration/service_configuration.h"
#include "services/configuration/service_descriptor.pb.h"

DEFINE_string(socket_address, "",
              "address:port to listen on. This flag overrides the value in service configuration, as specified by the "
              "--service_configuration flag. If no service configuration is specified, and this flag is not specified, "
              "the server will listen on 0.0.0.0 on its default port (service dependent).");

DEFINE_string(
    service_configuration, "",
    "Location (file path) of the service configuration. This configuration describes the port where each "
    "service should listen. If the location is not specified, or the service configuration does not contain a location "
    "for this service, then the server will listen on 0.0.0.0 on its default port (service dependent).");

namespace tvsc::service::configuration {

std::string determine_socket_address(std::string_view service_name, std::string_view end_point,
                                     const ServiceConfiguration& service_configuration,
                                     std::string_view default_value) {
  if (!end_point.empty()) {
    return std::string{end_point};
  } else {
    if (service_configuration.contains(service_name)) {
      const ServiceDescriptor& descriptor{service_configuration.get(service_name)};
      return descriptor.socket_address();
    }
  }

  return std::string{default_value};
}

std::string determine_socket_address(std::string_view service_name, std::string_view end_point,
                                     const std::filesystem::path& service_map_location,
                                     std::string_view default_value) {
  return determine_socket_address(service_name, end_point, ServiceConfiguration::load(service_map_location),
                                  default_value);
}

}  // namespace tvsc::service::configuration
