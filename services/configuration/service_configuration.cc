#include "services/configuration/service_configuration.h"

#include <filesystem>
#include <fstream>

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"
#include "services/configuration/service_descriptor.pb.h"

namespace tvsc::service::configuration {

void ServiceConfiguration::add(const ServiceDescriptor& service) {
  const auto [ignored, success] = services_.try_emplace(service.name(), service);
  if (!success) {
    // TODO(james): Implement allowing multiple services with the same name. This functionality could be used to
    // enable many interesting features in the proxy, including load balancing across multiple servers that publish
    // the same service.
    LOG(WARNING) << "Service with name '" << service.name()
                 << "' already configured. This is currently treated as a warning since we want to allow use cases "
                    "requiring this (such as load balancing) in the future.";
  }
}

void ServiceConfiguration::ingest(const ServiceDescriptors& descriptors) {
  for (const auto& service : descriptors.services()) {
    add(service);
  }
}

ServiceConfiguration ServiceConfiguration::load(const ServiceDescriptors& descriptors) {
  ServiceConfiguration result{};
  result.ingest(descriptors);
  return result;
}

ServiceConfiguration ServiceConfiguration::load(const std::filesystem::path& location) {
  ServiceDescriptors services{};
  {
    std::ifstream file{location};
    google::protobuf::io::IstreamInputStream proto_file{&file};
    if (!google::protobuf::TextFormat::Parse(&proto_file, &services)) {
      throw std::runtime_error(std::string{"Could not read service configuration file '"} + std::string{location} +
                               "'");
    }
  }

  return load(services);
}

}  // namespace tvsc::service::configuration
