#include "control/ranged_parameter_domain.h"

#include "gtest/gtest.h"

namespace tvsc::control {

TEST(RangedParameterDomainTest, CanDetectAllowedValue) {
  RangedParameterDomain<int> p{1, 3};
  EXPECT_TRUE(p.in_domain(1));
  EXPECT_TRUE(p.in_domain(2));
  EXPECT_TRUE(p.in_domain(3));
  EXPECT_FALSE(p.in_domain(0));
  EXPECT_FALSE(p.in_domain(4));
}

TEST(RangedParameterDomainTest, CanDetectAllowedValueWithHighExcluded) {
  RangedParameterDomain<int> p{1, 3, true, false};
  EXPECT_TRUE(p.in_domain(1));
  EXPECT_TRUE(p.in_domain(2));
  EXPECT_FALSE(p.in_domain(3));
  EXPECT_FALSE(p.in_domain(0));
  EXPECT_FALSE(p.in_domain(4));
}

TEST(RangedParameterDomainTest, CanDetectAllowedValueWithLowExcluded) {
  RangedParameterDomain<int> p{1, 3, false, true};
  EXPECT_FALSE(p.in_domain(1));
  EXPECT_TRUE(p.in_domain(2));
  EXPECT_TRUE(p.in_domain(3));
  EXPECT_FALSE(p.in_domain(0));
  EXPECT_FALSE(p.in_domain(4));
}

TEST(RangedParameterDomainTest, CanDetectAllowedValueExclusive) {
  RangedParameterDomain<int> p{1, 3, false, false};
  EXPECT_FALSE(p.in_domain(1));
  EXPECT_TRUE(p.in_domain(2));
  EXPECT_FALSE(p.in_domain(3));
  EXPECT_FALSE(p.in_domain(0));
  EXPECT_FALSE(p.in_domain(4));
}

TEST(RangedParameterDomainTest, CanCopy) {
  RangedParameterDomain<int> p1{1, 3};
  RangedParameterDomain<int> p2{p1};
  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_TRUE(p2.in_domain(2));
  EXPECT_TRUE(p2.in_domain(3));
  EXPECT_FALSE(p2.in_domain(0));
  EXPECT_FALSE(p2.in_domain(4));
}

TEST(RangedParameterDomainTest, CanMoveConstruct) {
  RangedParameterDomain<int> p1{-1, 1};
  RangedParameterDomain<int> p2{std::move(p1)};
  EXPECT_TRUE(p2.in_domain(-1));
  EXPECT_TRUE(p2.in_domain(0));
  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(-2));
}

TEST(RangedParameterDomainTest, CanAssign) {
  RangedParameterDomain<int> p1{1, 3};
  RangedParameterDomain<int> p2{0, 1};

  p2 = p1;

  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(0));
}

TEST(RangedParameterDomainTest, CanMoveAssign) {
  RangedParameterDomain<int> p1{1, 3};
  RangedParameterDomain<int> p2{0, 1};

  p2 = std::move(p1);

  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(0));
}

TEST(RangedParameterDomainTest, NonContinuousSizeIsNumberValues) {
  RangedParameterDomain<int> p1{1, 3};
  EXPECT_EQ(3, p1.size());
}

TEST(RangedParameterDomainTest, NonContinuousSizeIsNumberValuesMinusExcludedBoundaries) {
  RangedParameterDomain<int> p1{1, 3, true, false};
  RangedParameterDomain<int> p2{1, 3, false, true};
  RangedParameterDomain<int> p3{1, 3, false, false};
  EXPECT_EQ(2, p1.size());
  EXPECT_EQ(2, p2.size());
  EXPECT_EQ(1, p3.size());
}

TEST(RangedParameterDomainTest, ContinuousSizeIsDifference) {
  RangedParameterDomain<float> p1{1., 3.};
  EXPECT_EQ(2, p1.size());
}

TEST(RangedParameterDomainTest, ExcludedBoundariesIgnoredInContinousDomain) {
  RangedParameterDomain<float> p1{1., 3., true, false};
  RangedParameterDomain<float> p2{1., 3., false, true};
  RangedParameterDomain<float> p3{1., 3., false, false};
  EXPECT_EQ(2, p1.size());
  EXPECT_EQ(2, p2.size());
  EXPECT_EQ(2, p3.size());
}

}  // namespace tvsc::control
