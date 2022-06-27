#include "base/except.h"

#include <stdexcept>
#include <string>

#include "gtest/gtest.h"

namespace pack {

TEST(ExceptTest, HasMessage) {
  if constexpr (has_exception_support()) {
    constexpr char MESSAGE[] = "Some message";
    try {
      except<std::invalid_argument>(MESSAGE);
    } catch (const std::invalid_argument& e) {
      const std::string what{e.what()};
      EXPECT_NE(what.find(MESSAGE), std::string::npos);
    }
  }
}

TEST(ExceptTest, ThrowsExpectedException) {
  if constexpr (has_exception_support()) {
    EXPECT_THROW(except<std::invalid_argument>("Throwing std::invalid_argument"), std::invalid_argument);
    EXPECT_THROW(except<std::runtime_error>("Throwing std::runtime_error"), std::runtime_error);
  }
}

TEST(ExceptDeathTest, TerminatesWithoutExceptionSupport) {
  if constexpr (!has_exception_support()) {
    constexpr char MESSAGE[] = "Some message";
    EXPECT_EXIT(except<std::invalid_argument>(MESSAGE), testing::KilledBySignal(SIGABRT), MESSAGE);
  }
}

}  // namespace pack
