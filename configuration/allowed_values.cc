#include "configuration/allowed_values.h"

#include <string>
#include <variant>

namespace tvsc::configuration {

std::string to_string(const DiscreteValue& value) {
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

std::string to_string(const RangedValue& value) {
  std::string result{};
  if (const auto* v = std::get_if<ValueRange<int32_t>>(&value)) {
    result.append(to_string(*v));
  } else if (const auto* v = std::get_if<ValueRange<int64_t>>(&value)) {
    result.append(to_string(*v));
  } else if (const auto* v = std::get_if<ValueRange<float>>(&value)) {
    result.append(to_string(*v));
  } else if (const auto* v = std::get_if<ValueRange<double>>(&value)) {
    result.append(to_string(*v));
  }
  return result;
}

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
