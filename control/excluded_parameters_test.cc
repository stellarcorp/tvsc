#include "control/excluded_parameters.h"

#include <memory>

#include "control/discrete_parameter.h"
#include "control/parameter.h"
#include "control/parameters.h"
#include "control/ranged_parameter.h"
#include "gtest/gtest.h"

namespace tvsc::control {

TEST(ExcludedParameterTest, CanDetectAllowedValues) {
  std::unique_ptr<Parameter<int>> p1{new RangedParameter<int>(1, 5)};
  std::unique_ptr<Parameter<int>> p2{new DiscreteParameter<int>(2)};
  std::unique_ptr<Parameter<int>> combined{exclude<int>(std::move(p1), std::move(p2))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_FALSE(combined->is_allowed(2));
  EXPECT_TRUE(combined->is_allowed(3));
  EXPECT_TRUE(combined->is_allowed(5));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(6));
}

TEST(ExcludedParameterTest, CanUseRValues) {
  std::unique_ptr<Parameter<int>> combined{exclude<int>(std::make_unique<RangedParameter<int>>(1, 5), std::make_unique<DiscreteParameter<int>>(2), std::make_unique<DiscreteParameter<int>>(4))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_FALSE(combined->is_allowed(2));
  EXPECT_TRUE(combined->is_allowed(3));
  EXPECT_FALSE(combined->is_allowed(4));
  EXPECT_TRUE(combined->is_allowed(5));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(6));
}

}  // namespace tvsc::control
