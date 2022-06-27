#include "testing/test_temp_directory.h"

#include <cstdlib>
#include <filesystem>
#include <string_view>
#include <system_error>

#include "base/except.h"
#include "filesystem/utils.h"
#include "gtest/gtest.h"

namespace fs = std::filesystem;

namespace pack::testing {

TestTempDirectory* TestTempDirectory::instance_{nullptr};

TestTempDirectory* TestTempDirectory::getInstance() {
  if (instance_ == nullptr) {
    // Note: We do not need to delete this instance ourselves. We will register this instance via
    // ::testing::AddGlobalTestEnvironment() and gtest will take ownership of the memory.
    instance_ = new TestTempDirectory();
  }
  return instance_;
}

TestTempDirectory::~TestTempDirectory() { instance_ = nullptr; }

void TestTempDirectory::SetUp() {
  const char* test_tmp_dir{std::getenv("TEST_TMPDIR")};
  if (test_tmp_dir != nullptr) {
    test_directory_ = test_tmp_dir;
    requires_removal_ = false;
  } else {
    fs::path system_tmp_dir{fs::temp_directory_path()};
    test_directory_ = filesystem::generate_random_path(system_tmp_dir, "test-temp-directory");
    requires_removal_ = true;
  }
}

void TestTempDirectory::TearDown() {
  if (requires_removal_) {
    fs::remove_all(test_directory_);
  }
}

fs::path TestTempDirectory::createTestSubdirectory(std::string_view prefix, std::string_view suffix) {
  if (prefix == "" && suffix == "") {
    prefix = ::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name();
  }
  fs::path subdir{filesystem::generate_random_path(test_directory_, prefix, suffix)};
  std::error_code ec{};
  fs::create_directories(subdir, ec);
  if (ec) {
    std::string msg = std::string("Could not create directory '") + subdir.string() + "'";
    except<std::system_error>(msg, ec);
  }
  return subdir;
}

}  // namespace pack::testing
