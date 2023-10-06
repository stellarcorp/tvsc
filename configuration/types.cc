#include "configuration/types.h"

#include <string>
#include <string_view>
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
  } else if (const std::string_view* v = std::get_if<std::string_view>(&value)) {
    result.append(*v);
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

}  // namespace tvsc::configuration
