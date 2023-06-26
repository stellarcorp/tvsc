#pragma once

#include <string_view>

namespace tvsc::service::communications {

class Shakespeare final {
 private:
  std::string_view full_text_;
  std::string_view::iterator position_;

 public:
  Shakespeare();
  size_t get_next_line(unsigned char* buffer, size_t capacity);
};

}  // namespace tvsc::service::communications
