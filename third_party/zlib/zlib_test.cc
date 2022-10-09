#include "zlib.h"

#include <cstring>
#include <string>

#include "gtest/gtest.h"

TEST(ZlibTest, CanCompress) {
  constexpr size_t BUFFER_SIZE{256};
  constexpr char original[] = "The quick, brown fox jumped over the lazy dog.";
  const size_t original_length{std::strlen(original) + 1};
  ASSERT_GE(BUFFER_SIZE, original_length);

  Bytef compressed[BUFFER_SIZE];
  size_t compressed_length{BUFFER_SIZE};

  int err = compress(compressed, &compressed_length, reinterpret_cast<const Bytef*>(original), original_length);
  ASSERT_EQ(Z_OK, err);
  EXPECT_NE(std::strncmp(original, reinterpret_cast<const char*>(compressed), original_length), 0);

  char result[BUFFER_SIZE];
  size_t result_length{BUFFER_SIZE};

  err = uncompress(reinterpret_cast<Bytef*>(result), &result_length, compressed, compressed_length);
  ASSERT_EQ(Z_OK, err);

  EXPECT_EQ(result_length, original_length);
  EXPECT_EQ(std::strncmp(original, result, original_length), 0);
}
