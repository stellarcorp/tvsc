#include "services/echo/common/echo_service_location.h"

#include <string>

#include "services/configuration/service_location.h"

namespace tvsc::service::echo {

std::string get_echo_service_socket_address() {
  return configuration::determine_socket_address(ECHO_SERVICE_NAME, ECHO_SERVICE_DEFAULT_PORT);
}

}  // namespace tvsc::service::echo