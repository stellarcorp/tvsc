#pragma once

#include <string>

namespace tvsc::service::echo {

const char* const ECHO_SERVICE_NAME{"echo"};
const int ECHO_SERVICE_DEFAULT_PORT{50051};

std::string get_echo_service_bind_addr();

}  // namespace tvsc::service::echo
