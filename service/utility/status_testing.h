#pragma once

#include "grpcpp/support/status.h"
#include "gtest/gtest.h"

namespace tvsc::service::utility {

::testing::AssertionResult is_ok(const grpc::Status& status) {
  if (status.ok()) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure()
           << "(" << status.error_code() << ") -- " << status.error_message();
  }
}

}  // namespace tvsc::service::utility

// These are defined to look like macros (no namespace, all caps, etc.) to match the style of gtest
// ASSERT_* and EXPECT_* macros.
void ASSERT_OK(const grpc::Status& status) { ASSERT_TRUE(tvsc::service::utility::is_ok(status)); }
void EXPECT_OK(const grpc::Status& status) { EXPECT_TRUE(tvsc::service::utility::is_ok(status)); }
