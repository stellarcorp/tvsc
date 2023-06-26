#pragma once

#include <limits>
#include <string_view>

namespace tvsc::service::communications {

class Shakespeare final {
 private:
  std::string_view full_text_;
  std::string_view::iterator position_;

  size_t line_count_{std::numeric_limits<size_t>::max()};

 public:
  Shakespeare();
  size_t get_next_line(char* buffer, size_t capacity);
  size_t line_count();
};

}  // namespace tvsc::service::communications
