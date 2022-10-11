#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

#include "services/configuration/service_descriptor.pb.h"

namespace tvsc::service::configuration {

class ServiceConfiguration final {
 private:
  std::unordered_map<std::string, ServiceDescriptor> services_{};

 public:
  ServiceConfiguration() = default;
  ServiceConfiguration(const ServiceConfiguration& rhs) = default;
  ServiceConfiguration(ServiceConfiguration&& rhs) = default;
  ServiceConfiguration& operator=(const ServiceConfiguration& rhs) = default;
  ServiceConfiguration& operator=(ServiceConfiguration&& rhs) = default;

  void add(const ServiceDescriptor& descriptor);
  void ingest(const ServiceDescriptors& descriptors);

  bool contains(std::string_view service_name) const {
    return services_.find(std::string{service_name}) != services_.end();
  }

  std::vector<std::string> service_names() const {
    std::vector<std::string> names;
    names.reserve(services_.size());

    for (const auto& [name, _] : services_) {
      names.push_back(name);
    }
    return names;
  }

  const ServiceDescriptor& get(std::string_view service_name) const { return services_.at(std::string{service_name}); }

  /**
   * Load a configuration from a ServiceDescriptors proto.
   */
  static ServiceConfiguration load(const ServiceDescriptors& descriptors);

  /**
   * Load a configuration from a proto file.
   */
  static ServiceConfiguration load(const std::filesystem::path& location);
};

}  // namespace tvsc::service::configuration
