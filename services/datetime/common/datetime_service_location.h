#pragma once

#include <string>

#include "services/configuration/ports.h"

namespace tvsc::service::datetime {

const char* const DATETIME_SERVICE_NAME{"datetime"};
constexpr int DATETIME_SERVICE_DEFAULT_PORT{static_cast<int>(configuration::DefaultPort::DATETIME_SERVICE)};

std::string get_datetime_service_socket_address();

}  // namespace tvsc::service::datetime
