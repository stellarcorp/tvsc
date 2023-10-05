#include "configuration/system_definition.h"

#include <string>
#include <variant>

namespace tvsc::configuration {

std::string to_string(const AllowedValues::DiscreteValueT& value) {
  using std::to_string;
  std::string result{};
  if (const int32_t* v = std::get_if<int32_t>(&value)) {
    result.append(to_string(*v));
  } else if (const int64_t* v = std::get_if<int64_t>(&value)) {
    result.append(to_string(*v));
  } else if (const float* v = std::get_if<float>(&value)) {
    result.append(to_string(*v));
  } else if (const double* v = std::get_if<double>(&value)) {
    result.append(to_string(*v));
  }
  return result;
}

std::string to_string(const AllowedValues& values) {
  std::string result{};
  result.append("{");
  bool need_comma = false;
  for (const auto& value : values.enumerated_values()) {
    if (need_comma) {
      result.append(", ");
    }
    result.append(to_string(value));
    need_comma = true;
  }
  result.append("}");
  return result;
}

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
