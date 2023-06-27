#include <cstdio>

#include "base/initializer.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "testing/test_temp_directory.h"

GTEST_API_ int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Default logging configuration for tests.
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;

  tvsc::initialize(&argc, &argv);

  google::InstallFailureSignalHandler();

  ::testing::AddGlobalTestEnvironment(tvsc::testing::TestTempDirectory::getInstance());

  return RUN_ALL_TESTS();
}
