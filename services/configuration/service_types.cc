#include "services/configuration/service_types.h"

#include <string>

namespace tvsc::service::configuration {

std::string generate_service_type_from_name(const std::string& grpc_service_full_name) {
  // Service types must begin with an underscore.
  std::string result{"_"};
  result.append(grpc_service_full_name);

  // The generated gRPC service name is a dot-separated package and class name. mDNS requires that
  // we replace the dots with underscores.
  for (std::string::size_type i = 0; i < result.size() && i != std::string::npos;
       /* increment in loop via find() */) {
    i = result.find('.', i);
    if (i != std::string::npos) {
      result.replace(i, 1, 1, '_');
    }
  }

  // gRPC services use TCP.
  result.append("._tcp");

  return result;
}

}  // namespace tvsc::service::configuration
