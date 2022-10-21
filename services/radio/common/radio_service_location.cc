#include "services/radio/common/radio_service_location.h"

#include <string>

#include "services/configuration/service_location.h"

namespace tvsc::service::radio {

std::string get_radio_service_socket_address() {
  return configuration::determine_socket_address(RADIO_SERVICE_NAME, RADIO_SERVICE_DEFAULT_PORT);
}

}  // namespace tvsc::service::radio
