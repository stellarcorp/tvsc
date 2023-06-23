#include "base/initializer.h"

#include <filesystem>

#include "gflags/gflags.h"
#include "glog/logging.h"

namespace tvsc {

void initialize(int* argc, char*** argv, const std::string& log_dir) {
  // Try to find a log directory that will work, overriding log_dir parameter if necessary.
  std::filesystem::path log_path{log_dir};
  std::error_code ec{};
  std::filesystem::create_directories(log_path, ec);
  if (ec) {
    log_path = "/tmp/tvsc";
    std::filesystem::create_directories(log_path);
    if (ec) {
      // We couldn't fix the issue. Revert to user chosen directory. It will fail, but with a more
      // reasonable error message.
      log_path = log_dir;
    }
  }

  FLAGS_log_dir = log_path.string();
  google::InitGoogleLogging((*argv)[0]);
  gflags::ParseCommandLineFlags(argc, argv, true);
}

}  // namespace tvsc
