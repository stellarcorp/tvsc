#pragma once

#include <string>

#include "services/configuration/ports.h"

namespace tvsc::service::radio {

const char* const RADIO_SERVICE_NAME{"radio"};
constexpr int RADIO_SERVICE_DEFAULT_PORT{static_cast<int>(configuration::DefaultPort::RADIO_SERVICE)};

std::string get_radio_service_socket_address();

}  // namespace tvsc::service::radio
