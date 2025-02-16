#include "snark/snark.h"

#include <cstring>

#include "gtest/gtest.h"

namespace tvsc::snark {

TEST(SnarkTest, HasSomeSnark) { EXPECT_GT(num_snarks(), 0); }

TEST(SnarkTest, CanFetchSnark) { EXPECT_NE(nullptr, snark(0)); }

TEST(SnarkTest, CanFetchRandomSnark) { EXPECT_NE(nullptr, random_snark()); }

TEST(SnarkTest, AllSnarksHaveLength) {
  for (size_t i = 0; i < num_snarks(); ++i) {
    EXPECT_NE(nullptr, snark(i));
    EXPECT_GT(strlen(snark(i)), 0);
  }
}

TEST(SnarkTest, CanFetchManyRandomSnarks) {
  static constexpr size_t NUM_ITERATIONS{100};
  for (size_t i = 0; i < NUM_ITERATIONS; ++i) {
    const char* msg{random_snark()};
    EXPECT_NE(nullptr, msg);
    EXPECT_GT(strlen(msg), 0);
  }
}

}  // namespace tvsc::snark
