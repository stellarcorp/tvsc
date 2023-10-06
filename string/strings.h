#pragma once

#include <string>
#include <vector>

namespace tvsc::string {

template <typename T>
std::string to_string(const std::vector<T>& v) {
  std::string result{};
  bool need_comma{false};
  for (const auto i : v) {
    if (need_comma) {
      result.append(", ");
    }
    result.append(to_string(i));
    need_comma = true;
  }
  return result;
}

}  // namespace tvsc::string
