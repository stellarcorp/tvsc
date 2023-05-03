#include "hash/hash_combine.h"

#include <cstddef>

#include "gtest/gtest.h"

namespace tvsc::hash {

TEST(HashCombineTest, CanCombinePrimitiveTypes) {
  std::size_t value = hash_combine(0, 1, 2);
  EXPECT_NE(0, value);
}

}  // namespace tvsc::hash
