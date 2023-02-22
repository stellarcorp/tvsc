#pragma once

#include <filesystem>
#include <string_view>

#include "gtest/gtest.h"

namespace tvsc::testing {

class TestTempDirectory : public ::testing::Environment {
 private:
  TestTempDirectory() = default;

 public:
  static TestTempDirectory* getInstance();

  ~TestTempDirectory();

  void SetUp() override;
  void TearDown() override;

  const std::filesystem::path& getCurrentTestDirectory() { return test_directory_; }

  bool willRequireRemoval() { return requires_removal_; }

  std::filesystem::path createTestSubdirectory(std::string_view prefix = "",
                                               std::string_view suffix = "");

 private:
  static TestTempDirectory* instance_;

  std::filesystem::path test_directory_{};
  bool requires_removal_{false};
};

}  // namespace tvsc::testing
