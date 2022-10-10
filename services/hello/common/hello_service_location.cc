#include "services/hello/common/hello_service_location.h"

#include <string>

#include "services/configuration/service_location.h"

namespace tvsc::service::hello {

std::string get_hello_service_bind_addr() {
  return configuration::determine_service_addr(HELLO_SERVICE_NAME, HELLO_SERVICE_DEFAULT_PORT);
}

}  // namespace tvsc::service::hello
