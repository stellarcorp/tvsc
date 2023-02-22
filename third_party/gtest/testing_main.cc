#include <cstdio>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "testing/test_temp_directory.h"

#if GTEST_OS_ESP8266 || GTEST_OS_ESP32
#if GTEST_OS_ESP8266
extern "C" {
#endif
void setup() {
  testing::InitGoogleTest();
  google::InitGoogleLogging();
  google::InstallFailureSignalHandler();
  ::testing::AddGlobalTestEnvironment(pack::testing::TestTempDirectory::getInstance());
}

void loop() { RUN_ALL_TESTS(); }

#if GTEST_OS_ESP8266
}
#endif

#else

GTEST_API_ int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Default logging configuration for tests.
  FLAGS_logtostderr = true;
  FLAGS_stderrthreshold = 0;
  google::InitGoogleLogging(argv[0]);

  // But allow user to override via the command line.
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  google::InstallFailureSignalHandler();

  ::testing::AddGlobalTestEnvironment(tvsc::testing::TestTempDirectory::getInstance());

  return RUN_ALL_TESTS();
}

#endif
