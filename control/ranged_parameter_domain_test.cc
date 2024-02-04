#include "control/ranged_parameter_domain.h"

#include "gtest/gtest.h"

namespace tvsc::control {

TEST(RangedParameterDomainTest, CanDetectAllowedValue) {
  RangedParameterDomain<int> p{1, 3};
  EXPECT_TRUE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_TRUE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterDomainTest, CanDetectAllowedValueWithHighExcluded) {
  RangedParameterDomain<int> p{1, 3, true, false};
  EXPECT_TRUE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_FALSE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterDomainTest, CanDetectAllowedValueWithLowExcluded) {
  RangedParameterDomain<int> p{1, 3, false, true};
  EXPECT_FALSE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_TRUE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterDomainTest, CanDetectAllowedValueExclusive) {
  RangedParameterDomain<int> p{1, 3, false, false};
  EXPECT_FALSE(p.is_allowed(1));
  EXPECT_TRUE(p.is_allowed(2));
  EXPECT_FALSE(p.is_allowed(3));
  EXPECT_FALSE(p.is_allowed(0));
  EXPECT_FALSE(p.is_allowed(4));
}

TEST(RangedParameterDomainTest, CanCopy) {
  RangedParameterDomain<int> p1{1, 3};
  RangedParameterDomain<int> p2{p1};
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_TRUE(p2.is_allowed(2));
  EXPECT_TRUE(p2.is_allowed(3));
  EXPECT_FALSE(p2.is_allowed(0));
  EXPECT_FALSE(p2.is_allowed(4));
}

TEST(RangedParameterDomainTest, CanMoveConstruct) {
  RangedParameterDomain<int> p1{-1, 1};
  RangedParameterDomain<int> p2{std::move(p1)};
  EXPECT_TRUE(p2.is_allowed(-1));
  EXPECT_TRUE(p2.is_allowed(0));
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(-2));
}

TEST(RangedParameterDomainTest, CanAssign) {
  RangedParameterDomain<int> p1{1, 3};
  RangedParameterDomain<int> p2{0, 1};

  p2 = p1;

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(RangedParameterDomainTest, CanMoveAssign) {
  RangedParameterDomain<int> p1{1, 3};
  RangedParameterDomain<int> p2{0, 1};

  p2 = std::move(p1);

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

}  // namespace tvsc::control
