#pragma once

#include <string>

#include "buffer/store.h"

namespace tvsc::buffer {

std::string to_string(const IsStore auto& s) {
  using std::to_string;
  std::string result{};
  if (s.size() > 0) {
    result += "[\n";
    for (const auto& v : s) {
      result += "\t";
      result += to_string(v);
      result += ",\n";
    }
    result += "]";
  } else {
    result += "[]";
  }
  return result;
}

}  // namespace tvsc::buffer
