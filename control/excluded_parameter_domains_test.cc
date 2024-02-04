#include "control/excluded_parameter_domains.h"

#include <memory>

#include "control/discrete_parameter_domain.h"
#include "control/parameter_domain.h"
#include "control/parameter_domains.h"
#include "control/ranged_parameter_domain.h"
#include "gtest/gtest.h"

namespace tvsc::control {

TEST(ExcludedParameterDomainTest, CanDetectAllowedValues) {
  std::unique_ptr<ParameterDomain<int>> p1{new RangedParameterDomain<int>(1, 5)};
  std::unique_ptr<ParameterDomain<int>> p2{new DiscreteParameterDomain<int>(2)};
  std::unique_ptr<ParameterDomain<int>> combined{exclude<int>(std::move(p1), std::move(p2))};
  EXPECT_TRUE(combined->in_domain(1));
  EXPECT_FALSE(combined->in_domain(2));
  EXPECT_TRUE(combined->in_domain(3));
  EXPECT_TRUE(combined->in_domain(5));
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(6));
}

TEST(ExcludedParameterDomainTest, CanUseRValues) {
  std::unique_ptr<ParameterDomain<int>> combined{
      exclude<int>(std::make_unique<RangedParameterDomain<int>>(1, 5),
                   std::make_unique<DiscreteParameterDomain<int>>(2),
                   std::make_unique<DiscreteParameterDomain<int>>(4))};
  EXPECT_TRUE(combined->in_domain(1));
  EXPECT_FALSE(combined->in_domain(2));
  EXPECT_TRUE(combined->in_domain(3));
  EXPECT_FALSE(combined->in_domain(4));
  EXPECT_TRUE(combined->in_domain(5));
  EXPECT_FALSE(combined->in_domain(0));
  EXPECT_FALSE(combined->in_domain(6));
}

}  // namespace tvsc::control
