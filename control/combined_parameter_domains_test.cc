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
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(3));
}

TEST(CombinedParameterDomainTest, CanUseRValues) {
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(3));
}

TEST(CombinedParameterDomainTest, CanCombineSeveralParameterDomains) {
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2),
                   new DiscreteParameterDomain<int>(3), new DiscreteParameterDomain<int>(4))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_TRUE(combined->is_allowed(3));
  EXPECT_TRUE(combined->is_allowed(4));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(5));
}

TEST(CombinedParameterDomainTest, CanCombineRangedAndDiscrete) {
  std::unique_ptr<ParameterDomain<int>> combined{
      combine<int>(new DiscreteParameterDomain<int>(1), new DiscreteParameterDomain<int>(2),
                   new RangedParameterDomain<int>(3, 4))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_TRUE(combined->is_allowed(3));
  EXPECT_TRUE(combined->is_allowed(4));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(5));
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
    EXPECT_TRUE(combined->is_allowed(i)) << "i: " << i;
  }
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(11));
}

}  // namespace tvsc::control
