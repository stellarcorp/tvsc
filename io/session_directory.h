#pragma once

#include <filesystem>
#include <string>

#include "time/chrono_utils.h"

namespace tvsc::io {

class SessionDirectory final {
 private:
  std::filesystem::path directory_{"/tmp/tvsc"};

  void ensure_directory_exists();

 public:
  SessionDirectory() { ensure_directory_exists(); }

  explicit SessionDirectory(const std::filesystem::path& directory) noexcept
      : directory_(directory) {
    ensure_directory_exists();
  }

  std::filesystem::path contextualize_filename(const std::string& filename) noexcept {
    std::filesystem::path result{directory_};
    result.append(filename);
    return result;
  }

  std::filesystem::path create_temp_filename(const std::string& prefix = "",
                                             const std::string& suffix = "") noexcept;

  template <typename ClockType>
  std::filesystem::path create_timestamp_filename(ClockType::time_point timestamp,
                                                  const std::string& prefix = "",
                                                  const std::string& suffix = "") noexcept {
    std::filesystem::path result{directory_};
    result.append(prefix + format_timestamp(timestamp) + suffix);
    return result;
  }

  template <typename ClockType>
  std::filesystem::path create_timestamp_filename(const std::string& prefix = "",
                                                  const std::string& suffix = "") noexcept {
    return create_timestamp_filename(ClockType::now(), prefix, suffix);
  }
};

}  // namespace tvsc::io
