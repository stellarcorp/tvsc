#include <stdexcept>

#include "gtest/gtest.h"

void throws_exception() {
  throw std::exception();
}

void throws_something() {
  throw "something";
}

TEST(GTestTest, CanCompile) {
  EXPECT_TRUE(true);
}

TEST(GTestTest, CanExpectOnExceptions) {
  EXPECT_THROW(throws_exception(), std::exception);
}

TEST(GTestTest, CanExpectOnNonstandardExceptions) {
  EXPECT_ANY_THROW(throws_something());
}
