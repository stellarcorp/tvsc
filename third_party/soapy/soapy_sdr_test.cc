#include "gtest/gtest.h"
#include "SoapySDR/Version.h"

// Trivial test to verify compilation and linking.
TEST(SoapySDR, CanGetVersionString) {
  EXPECT_NE(SoapySDR_getAPIVersion(), nullptr);
}
