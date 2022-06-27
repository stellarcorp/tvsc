#include "filesystem/utils.h"

#include <array>
#include <filesystem>
#include <random>
#include <string>
#include <string_view>

namespace pack::filesystem {

namespace fs = std::filesystem;

/**
 * Generate a random string of the requested length using the characters [a-zA-Z0-9].
 */
std::string random_string(size_t length) {
  static constexpr std::array<char, 10 + 2 * 26> chars{
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',                 //
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',  //
      'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',  //
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',  //
      'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',  //
  };

  static std::default_random_engine rng{};
  static std::uniform_int_distribution<> dist(0, chars.size() - 1);

  std::string result(length, '\0');
  for (size_t i = 0; i < length; ++i) {
    result[i] = chars[dist(rng)];
  }
  return result;
}

fs::path generate_random_path(const fs::path& base_dir, std::string_view prefix, std::string_view suffix) {
  std::string filename{prefix};
  filename.append(random_string(10)).append(suffix);
  return base_dir / filename;
}

}  // namespace pack::filesystem
