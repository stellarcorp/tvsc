#include "configuration/system_definition.h"

#include <string>
#include <variant>

namespace tvsc::configuration {

std::string to_string(const Function& function) {
  using std::to_string;
  std::string result{};
  result.append("{ id: ").append(to_string(function.identifier()));
  result.append(", allowed_values: ").append(to_string(function.allowed_values()));
  result.append("}");
  return result;
}

std::string to_string(const Component& component) {
  using std::to_string;
  std::string result{};
  result.append("{ id: ").append(to_string(component.identifier())).append(", functions: {");
  for (const auto& function : component.functions()) {
    result.append(to_string(function));
  }
  result.append("}");
  return result;
}

std::string to_string(const System& system) {
  using std::to_string;
  std::string result{};
  result.append("{id: ").append(to_string(system.identifier())).append(", subsystems: {");
  for (const auto& subsystem : system.subsystems()) {
    result.append(to_string(subsystem));
    result.append("\n");
  }
  result.append("}");
  result.append(", components: {");
  for (const auto& component : system.components()) {
    result.append(to_string(component));
    result.append("\n");
  }
  result.append("},");
  result.append("}");
  return result;
}

}  // namespace tvsc::configuration
