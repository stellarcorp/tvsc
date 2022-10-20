#pragma once

#include <string>

#include "services/configuration/ports.h"

namespace tvsc::service::hello {

const char* const HELLO_SERVICE_NAME{"hello"};
constexpr int HELLO_SERVICE_DEFAULT_PORT{static_cast<int>(configuration::DefaultPort::HELLO_SERVICE)};

std::string get_hello_service_socket_address();

}  // namespace tvsc::service::hello
