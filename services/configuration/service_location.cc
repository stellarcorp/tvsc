#include "services/configuration/service_location.h"

#include <filesystem>
#include <string>
#include <string_view>

#include "services/configuration/service_configuration.h"

DEFINE_string(bind_addr, "",
              "address:port to listen on. This flag overrides the value in service configuration, as specified by the "
              "--service_configuration flag. If no service configuration is specified, and this flag is not specified, "
              "the server will listen on 127.0.0.1 on its default port (service dependent).");

DEFINE_string(
    service_configuration, "",
    "Location (file path) of the service configuration. This configuration describes the port where each "
    "service should listen. If the location is not specified, or the service configuration does not contain a location "
    "for this service, then the server will listen on 127.0.0.1 on its default port (service dependent).");

namespace tvsc::service::configuration {
std::string determine_service_addr(std::string_view service_name, std::string_view bind_addr,
                                   const ServiceConfiguration& service_configuration, std::string_view default_value) {
  if (!bind_addr.empty()) {
    return std::string{bind_addr};
  } else {
    std::string result{service_configuration.lookup_bind_addr(service_name)};
    if (!result.empty()) {
      return result;
    }
  }

  return std::string{default_value};
}

std::string determine_service_addr(std::string_view service_name, std::string_view bind_addr,
                                   const std::filesystem::path& service_map_location, std::string_view default_value) {
  return determine_service_addr(service_name, bind_addr, ServiceConfiguration::load(service_map_location),
                                default_value);
}

}  // namespace tvsc::service::configuration
