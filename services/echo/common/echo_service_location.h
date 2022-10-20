#pragma once

#include <string>

#include "services/configuration/ports.h"

namespace tvsc::service::echo {

const char* const ECHO_SERVICE_NAME{"echo"};
constexpr int ECHO_SERVICE_DEFAULT_PORT{static_cast<int>(configuration::DefaultPort::ECHO_SERVICE)};

std::string get_echo_service_socket_address();

}  // namespace tvsc::service::echo
