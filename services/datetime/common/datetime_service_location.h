#pragma once

#include <string>

namespace tvsc::service::datetime {

const char* const DATETIME_SERVICE_NAME{"datetime"};
const int DATETIME_SERVICE_DEFAULT_PORT{50052};

std::string get_datetime_service_socket_address();

}  // namespace tvsc::service::datetime
