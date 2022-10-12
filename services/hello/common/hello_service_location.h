#pragma once

#include <string>

namespace tvsc::service::hello {

const char* const HELLO_SERVICE_NAME{"hello"};
const int HELLO_SERVICE_DEFAULT_PORT{50050};

std::string get_hello_service_socket_address();

}  // namespace tvsc::service::hello
