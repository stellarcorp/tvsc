#include <string>
#include <vector>

#include "absl/strings/str_join.h"
#include "gtest/gtest.h"

TEST(AbslTest, CanJoinStrings) {
  std::vector<std::string> v = {"foo", "bar", "baz"};
  const std::string s{absl::StrJoin(v, "-")};
  EXPECT_EQ("foo-bar-baz", s);
}
