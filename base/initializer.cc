#include "base/initializer.h"

#include <filesystem>

#include "gflags/gflags.h"
#include "glog/logging.h"

namespace tvsc {

void initialize(int* argc, char*** argv) {
  gflags::ParseCommandLineFlags(argc, argv, true);
  google::InitGoogleLogging((*argv)[0]);
}

}  // namespace tvsc
