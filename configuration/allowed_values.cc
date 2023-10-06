#include "configuration/allowed_values.h"

#include <string>
#include <string_view>
#include <variant>

#include "configuration/types.h"

namespace tvsc::configuration {

std::string to_string(const AllowedValues& values) {
  std::string result{};
  result.append("{\nenumerated: {");
  bool need_comma = false;
  for (const auto& value : values.enumerated_values()) {
    if (need_comma) {
      result.append(", ");
    }
    result.append(to_string(value));
    need_comma = true;
  }
  result.append("}, ranged: {");
  need_comma = false;
  for (const auto& value : values.ranged_values()) {
    if (need_comma) {
      result.append(", ");
    }
    result.append(to_string(value));
    need_comma = true;
  }
  result.append("}\n}");
  return result;
}

}  // namespace tvsc::configuration
