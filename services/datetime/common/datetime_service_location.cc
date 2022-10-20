#include "services/datetime/common/datetime_service_location.h"

#include <string>

#include "services/configuration/service_location.h"

namespace tvsc::service::datetime {

std::string get_datetime_service_socket_address() {
  return configuration::determine_socket_address(DATETIME_SERVICE_NAME, DATETIME_SERVICE_DEFAULT_PORT);
}

}  // namespace tvsc::service::datetime
