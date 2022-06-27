#pragma once

#include <filesystem>
#include <string_view>

namespace pack::filesystem {

/**
 * Generate a random path under base_dir. The filename of the path will start with prefix and end with suffix. The path
 * may or may not already exist. This function is helpful for creating temporary files or directories where the behavior
 * of std::tmpfile() and similar functions are unacceptable and the security issues are not relevant (ie, tests).
 */
std::filesystem::path generate_random_path(const std::filesystem::path& base_dir, std::string_view prefix = "",
                                           std::string_view suffix = "");

}  // namespace pack::filesystem
