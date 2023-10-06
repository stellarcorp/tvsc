#include "configuration/system_definition.h"

#include <string>
#include <variant>

#include "configuration/allowed_values.h"

namespace tvsc::configuration {

std::string to_string(const Function& function) {
  using std::to_string;
  std::string result{};
  result.append("{ id: ").append(to_string(function.id()));
  result.append(", allowed_values: ").append(to_string(function.allowed_values()));
  result.append("}");
  return result;
}

std::string to_string(const Property& property) {
  std::string result{};
  result.append("{ id: ").append(to_string(property.id()));
  result.append(", value: ").append(to_string(property.value()));
  result.append("}");
  return result;
}

std::string to_string(const System& system) {
  using std::to_string;
  std::string result{};
  result.append("{id: ").append(to_string(system.id())).append(", subsystems: {");
  for (const auto& subsystem : system.subsystems()) {
    result.append(to_string(subsystem));
    result.append("\n");
  }
  result.append("}");
  result.append(", properties: {");
  for (const auto& property : system.properties()) {
    result.append(to_string(property));
    result.append("\n");
  }
  result.append("},");
  result.append(", functions: {");
  for (const auto& function : system.functions()) {
    result.append(to_string(function));
    result.append("\n");
  }
  result.append("},");
  result.append("}");
  return result;
}

}  // namespace tvsc::configuration
