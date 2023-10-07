#include "configuration/configuration.h"

#include <string>

#include "string/strings.h"

namespace tvsc::configuration {

bool is_valid_configuration(const System& system, const SystemConfiguration& configuration) {
  for (const auto& setting : configuration.settings()) {
    const Function* function = system.get_function(setting.id());
    if (function == nullptr) {
      return false;
    }

    const auto& value = setting.value();
    if (!function->is_allowed(value)) {
      return false;
    }
  }

  for (const auto& config_subsystem : configuration.subsystems()) {
    const auto* system_subsystem = system.get_subsystem(config_subsystem.id());
    if (system_subsystem == nullptr) {
      return false;
    }

    if (!is_valid_configuration(*system_subsystem, config_subsystem)) {
      return false;
    }
  }

  return true;
}

std::string to_string(const Setting& setting) {
  std::string result{};
  result.append("{ ")
      .append(std::to_string(setting.id()))
      .append(": ")
      .append(to_string(setting.value()))
      .append(" }");
  return result;
}

std::string to_string(const SystemConfiguration& configuration) {
  std::string result{};
  result.append("{ id: ")
      .append(std::to_string(configuration.id()))
      .append(", subsystems: ")
      .append(tvsc::string::to_string(configuration.subsystems()))
      .append(", settings: ")
      .append(tvsc::string::to_string(configuration.settings()))
      .append(" }");
  return result;
}

}  // namespace tvsc::configuration
