#include "control/ranged_parameter_domain.h"

#include "gtest/gtest.h"

namespace tvsc::control {

TEST(RangedParameterTest, CanDetectAllowedValue) {
  RangedParameter<int> p{1, 3};
  EXPECT_TRUE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_TRUE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterTest, CanDetectAllowedValueWithHighExcluded) {
  RangedParameter<int> p{1, 3, true, false};
  EXPECT_TRUE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_FALSE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterTest, CanDetectAllowedValueWithLowExcluded) {
  RangedParameter<int> p{1, 3, false, true};
  EXPECT_FALSE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_TRUE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterTest, CanDetectAllowedValueExclusive) {
  RangedParameter<int> p{1, 3, false, false};
  EXPECT_FALSE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_FALSE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterTest, CanCopy) {
  RangedParameter<int> p1{1, 3};
  RangedParameter<int> p2{p1};
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_TRUE(p2.is_allowed(2));
  EXPECT_TRUE(p2.is_allowed(3));
  EXPECT_FALSE(p2.is_allowed(0));
  EXPECT_FALSE(p2.is_allowed(4));
}

TEST(RangedParameterTest, CanMoveConstruct) {
  RangedParameter<int> p1{-1, 1};
  RangedParameter<int> p2{std::move(p1)};
  EXPECT_TRUE(p2.is_allowed(-1));
  EXPECT_TRUE(p2.is_allowed(0));
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(-2));
}

TEST(RangedParameterTest, CanAssign) {
  RangedParameter<int> p1{1, 3};
  RangedParameter<int> p2{0, 1};

  p2 = p1;

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(RangedParameterTest, CanMoveAssign) {
  RangedParameter<int> p1{1, 3};
  RangedParameter<int> p2{0, 1};

  p2 = std::move(p1);

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

}  // namespace tvsc::control
