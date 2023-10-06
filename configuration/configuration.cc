#include "configuration/configuration.h"

#include <string>

#include "string/strings.h"

namespace tvsc::configuration {

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
