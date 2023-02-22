#include "filesystem/utils.h"

#include <filesystem>
#include <string>

#include "gtest/gtest.h"

namespace tvsc::filesystem {

namespace fs = std::filesystem;

TEST(GenerateRandomPathTest, CanGeneratePath) {
  const fs::path base_dir{"/some_base_dir"};
  fs::path path = generate_random_path(base_dir);
  EXPECT_EQ(base_dir, path.parent_path());
  EXPECT_FALSE(path.filename().empty());
  EXPECT_GT(path.filename().string().size(), 4);
}

TEST(GenerateRandomPathTest, PathsAreDifferentBetweenCalls) {
  const fs::path base_dir{"/"};
  fs::path path1 = generate_random_path(base_dir);
  ASSERT_FALSE(path1.filename().empty());
  fs::path path2 = generate_random_path(base_dir);
  ASSERT_FALSE(path2.filename().empty());
  EXPECT_NE(path1.filename(), path2.filename());
}

TEST(GenerateRandomPathTest, CanGeneratePathUsesBaseDir) {
  const fs::path base_dir{"/some_base_dir/with_another_subdir"};
  fs::path path = generate_random_path(base_dir);
  EXPECT_EQ(base_dir, path.parent_path());
}

TEST(GenerateRandomPathTest, CanGeneratePathWithPrefix) {
  const fs::path base_dir{"/some_base_dir"};
  const std::string prefix{"prefix-"};
  fs::path path = generate_random_path(base_dir, prefix);
  EXPECT_EQ(0, path.filename().string().find(prefix));
}

TEST(GenerateRandomPathTest, CanGeneratePathWithSuffix) {
  const fs::path base_dir{"/some_base_dir"};
  const std::string suffix{".some_suffix"};
  const fs::path path = generate_random_path(base_dir, "", suffix);
  const std::string filename{path.filename().string()};
  EXPECT_EQ(filename.length() - suffix.length(), filename.rfind(suffix));
}

}  // namespace tvsc::filesystem
