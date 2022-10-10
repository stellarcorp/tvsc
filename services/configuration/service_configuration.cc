#include "services/configuration/service_configuration.h"

#include <filesystem>
#include <fstream>

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "services/configuration/service_descriptor.pb.h"

namespace tvsc::service::configuration {

ServiceConfiguration ServiceConfiguration::load(const std::filesystem::path& location) {
  ServiceDescriptors services{};
  {
    std::ifstream file{location};
    google::protobuf::io::IstreamInputStream proto_file{&file};
    if (!google::protobuf::TextFormat::Parse(&proto_file, &services)) {
      throw std::runtime_error(std::string{"Could not read service configuration file '"} + std::string{location} + "'");
    }
  }

  ServiceConfiguration result{};
  for (const auto& service : services.services()) {
    result.insert(service.name(), service.bind_addr());
  }
  return result;
}

}  // namespace tvsc::service::configuration
