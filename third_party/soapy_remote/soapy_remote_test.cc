#include "gtest/gtest.h"
#include "SoapyInfoUtils.hpp"

// Trivial test to verify compilation and linking.
TEST(SoapyRemote, CanGetVersionString) {
  EXPECT_FALSE(SoapyInfo::getServerVersion().empty());
}
