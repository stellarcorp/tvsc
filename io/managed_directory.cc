#include "io/managed_directory.h"

#include <filesystem>

#include "random/random.h"
#include "string/base64.h"

namespace tvsc::io {

void ManagedDirectory::ensure_directory_exists() {
  std::filesystem::create_directories(directory_);
}

std::filesystem::path ManagedDirectory::create_temp_filename(const std::string& prefix,
                                                             const std::string& suffix) noexcept {
  auto random{tvsc::random::generate_random_value<uint64_t>()};
  return std::filesystem::path{prefix + tvsc::string::Base64::encode(random) + suffix};
}

}  // namespace tvsc::io
