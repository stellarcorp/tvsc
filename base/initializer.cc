#include "base/initializer.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

namespace tvsc {

void initialize(int* argc, char*** argv, const std::string& log_dir) {
  FLAGS_log_dir = log_dir;
  google::InitGoogleLogging((*argv)[0]);
  gflags::ParseCommandLineFlags(argc, argv, true);
}

}  // namespace tvsc
