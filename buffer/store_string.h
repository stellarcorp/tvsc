#pragma once

#include <iostream>
#include <string>

#include "buffer/store.h"
#include "buffer/store_interface.h"

namespace tvsc::buffer {

std::string to_string(const IsStore auto& s) {
  using std::to_string;
  std::string result{};
  result.append("<")
      .append(to_string(s.size()))
      .append("/")
      .append(to_string(s.capacity()))
      .append("> ");
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

std::ostream& operator<<(std::ostream& os, const IsStore auto& s) {
  os << to_string(s);
  return os;
}

template <IsStore S, bool is_const>
std::string to_string(const internal::RandomAccessStoreIterator<S, is_const>& iter) {
  using std::to_string;
  std::string result{"<@"};
  result += to_string(iter.pos());
  result += ">";
  return result;
}

template <IsStore S, bool is_const>
std::ostream& operator<<(std::ostream& os,
                         const internal::RandomAccessStoreIterator<S, is_const>& iter) {
  os << to_string(iter);
  return os;
}

}  // namespace tvsc::buffer
