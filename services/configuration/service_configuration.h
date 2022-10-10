#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>

namespace tvsc::service::configuration {

class ServiceConfiguration final {
 private:
  std::unordered_map<std::string, std::string> services_{};

 public:
  ServiceConfiguration() = default;
  ServiceConfiguration(const ServiceConfiguration& rhs) = default;
  ServiceConfiguration(ServiceConfiguration&& rhs) = default;

  ServiceConfiguration& operator=(const ServiceConfiguration& rhs) = default;
  ServiceConfiguration& operator=(ServiceConfiguration&& rhs) = default;

  /**
   * Sets the bind_addr for service_name, updating a previous entry if one exists. Returns true if an insert was
   * required; false if an existing entry was updated and no insert was needed.
   */
  bool upsert(std::string_view service_name, std::string_view bind_addr) {
    const auto result{services_.insert_or_assign(std::string{service_name}, bind_addr)};
    return result.second;
  }

  /**
   * Inserts the bind_addr for a new service_name, throwing an exception if service_name already exists.
   */
  void insert(std::string_view service_name, std::string_view bind_addr) {
    const auto result{services_.try_emplace(std::string{service_name}, bind_addr)};
    if (!result.second) {
      throw std::logic_error("ServiceConfiguration::insert(): service_name '" + std::string{service_name} +
                             "' already exists in ServiceConfiguration");
    }
  }

  /**
   * Remove any bind_addr for service_name, if one exists. Returns true if service_name already had a bind_addr and was
   * removed; false if no such entry existed.
   */
  bool remove(std::string_view service_name) {
    const auto result{services_.erase(std::string{service_name})};
    if (result == 0) {
      return false;
    } else {
      return true;
    }
  }

  bool contains(std::string_view service_name) const {
    return services_.find(std::string{service_name}) != services_.end();
  }

  /**
   * Looks up the bind_addr for service_name and returning that value. Returns the empty string if no such service_name
   * exists.
   */
  std::string lookup_bind_addr(std::string_view service_name) const {
    auto iter{services_.find(std::string{service_name})};
    if (iter != services_.end()) {
      return iter->second;
    } else {
      return "";
    }
  }

  /**
   * Load a configuration from a proto file.
   */
  static ServiceConfiguration load(const std::filesystem::path& location);
};

}  // namespace tvsc::service::configuration
