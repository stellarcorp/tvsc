#include "control/combined_parameter_domains.h"

#include <memory>

#include "control/discrete_parameter_domain.h"
#include "control/parameter_domain.h"
#include "control/parameter_domains.h"
#include "control/ranged_parameter_domain.h"
#include "gtest/gtest.h"

namespace tvsc::control {

TEST(CombinedParameterDomainTest, CanDetectAllowedValue) {
  std::unique_ptr<ParameterDomain<int>> p1{new DiscreteParameterDomain<int>(1)};
  std::unique_ptr<ParameterDomain<int>> p2{new DiscreteParameterDomain<int>(2)};
  std::unique_ptr<ParameterDomain<int>> combined{combine<int>(std::move(p1), std::move(p2))};
  EXPECT_TRUE(combined->in_domain(1));
  EXPECT_TRUE(combined->in_domain(2));
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(3));
}

TEST(CombinedParameterDomainTest, CanUseRValues) {
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2))};
  EXPECT_TRUE(combined->in_domain(1));
  EXPECT_TRUE(combined->in_domain(2));
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(3));
}

TEST(CombinedParameterDomainTest, CanCombineSeveralParameterDomains) {
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2),
                   new DiscreteParameterDomain<int>(3), new DiscreteParameterDomain<int>(4))};
  EXPECT_TRUE(combined->in_domain(1));
  EXPECT_TRUE(combined->in_domain(2));
  EXPECT_TRUE(combined->in_domain(3));
  EXPECT_TRUE(combined->in_domain(4));
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(5));
}

TEST(CombinedParameterDomainTest, CanCombineRangedAndDiscrete) {
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2),
                   new RangedParameterDomain<int>(3, 4))};
  EXPECT_TRUE(combined->in_domain(1));
  EXPECT_TRUE(combined->in_domain(2));
  EXPECT_TRUE(combined->in_domain(3));
  EXPECT_TRUE(combined->in_domain(4));
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(5));
}

TEST(CombinedParameterDomainTest, CanCombineCombinations) {
  std::unique_ptr<ParameterDomain<int>> combined1{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2),
                   new RangedParameterDomain<int>(3, 4))};
  std::unique_ptr<ParameterDomain<int>> combined2{
      combine<int>(new DiscreteParameterDomain<int>(5), new DiscreteParameterDomain<int>(6),
                   new RangedParameterDomain<int>(7, 10))};
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(std::move(combined1), std::move(combined2))};
  for (int i = 1; i <= 10; ++i) {
    EXPECT_TRUE(combined->in_domain(i)) << "i: " << i;
  }
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(11));
}

TEST(CombinedParameterDomainTest, SizeIsSumOfSizes) {
  std::unique_ptr<ParameterDomain<int>> p1{new DiscreteParameterDomain<int>(1)};
  std::unique_ptr<ParameterDomain<int>> p2{new DiscreteParameterDomain<int>(2)};
  std::unique_ptr<ParameterDomain<int>> p3{new RangedParameterDomain<int>(5, 10)};

  ASSERT_EQ(1, p1->size());
  ASSERT_EQ(1, p2->size());
  ASSERT_EQ(6, p3->size());

  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(std::move(p1), std::move(p2), std::move(p3))};

  EXPECT_EQ(8, combined->size());
}

TEST(CombinedParameterDomainTest, SizePurposefullyDoubleCountsOverlapInRangedDomains) {
  std::unique_ptr<ParameterDomain<int>> p1{new RangedParameterDomain<int>(5, 8)};
  std::unique_ptr<ParameterDomain<int>> p2{new RangedParameterDomain<int>(7, 10)};

  ASSERT_EQ(4, p1->size());
  ASSERT_EQ(4, p2->size());

  std::unique_ptr<ParameterDomain<int>> combined{combine<int>(std::move(p1), std::move(p2))};

  EXPECT_EQ(8, combined->size());
}

}  // namespace tvsc::control
