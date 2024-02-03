#include "control/parameter.h"
#include "control/discrete_parameter.h"
#include "control/ranged_parameter.h"

#include <memory>

#include "gtest/gtest.h"

namespace tvsc::control {

  TEST(CombinedParameterTest, CanDetectAllowedValue) {
    std::unique_ptr<Parameter<int>> p1{new DiscreteParameter<int>(1)};
    std::unique_ptr<Parameter<int>> p2{new DiscreteParameter<int>(2)};
    std::unique_ptr<Parameter<int>> combined{combine<int>(std::move(p1), std::move(p2))};
    EXPECT_TRUE(combined->is_allowed(1));
    EXPECT_TRUE(combined->is_allowed(2));
    EXPECT_FALSE(combined->is_allowed(0));
    EXPECT_FALSE(combined->is_allowed(3));
  }

  TEST(CombinedParameterTest, CanUseRValues) {
    std::unique_ptr<Parameter<int>> combined{combine<int>(new DiscreteParameter<int>(1), new DiscreteParameter<int>(2))};
    EXPECT_TRUE(combined->is_allowed(1));
    EXPECT_TRUE(combined->is_allowed(2));
    EXPECT_FALSE(combined->is_allowed(0));
    EXPECT_FALSE(combined->is_allowed(3));
  }

  TEST(CombinedParameterTest, CanCombineSeveralParameters) {
    std::unique_ptr<Parameter<int>> combined{combine<int>(new DiscreteParameter<int>(1), new DiscreteParameter<int>(2), new DiscreteParameter<int>(3), new DiscreteParameter<int>(4))};
    EXPECT_TRUE(combined->is_allowed(1));
    EXPECT_TRUE(combined->is_allowed(2));
    EXPECT_TRUE(combined->is_allowed(3));
    EXPECT_TRUE(combined->is_allowed(4));
    EXPECT_FALSE(combined->is_allowed(0));
    EXPECT_FALSE(combined->is_allowed(5));
  }

}
