#include "control/combined_parameters.h"

#include <memory>

#include "control/discrete_parameter.h"
#include "control/parameter.h"
#include "control/parameters.h"
#include "control/ranged_parameter.h"
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
  std::unique_ptr<Parameter<int>> combined{
      combine<int>(new DiscreteParameter<int>(1), new DiscreteParameter<int>(2))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(3));
}

TEST(CombinedParameterTest, CanCombineSeveralParameters) {
  std::unique_ptr<Parameter<int>> combined{
      combine<int>(new DiscreteParameter<int>(1), new DiscreteParameter<int>(2),
                   new DiscreteParameter<int>(3), new DiscreteParameter<int>(4))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_TRUE(combined->is_allowed(3));
  EXPECT_TRUE(combined->is_allowed(4));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(5));
}

TEST(CombinedParameterTest, CanCombineRangedAndDiscrete) {
  std::unique_ptr<Parameter<int>> combined{combine<int>(new DiscreteParameter<int>(1),
                                                        new DiscreteParameter<int>(2),
                                                        new RangedParameter<int>(3, 4))};
  EXPECT_TRUE(combined->is_allowed(1));
  EXPECT_TRUE(combined->is_allowed(2));
  EXPECT_TRUE(combined->is_allowed(3));
  EXPECT_TRUE(combined->is_allowed(4));
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(5));
}

TEST(CombinedParameterTest, CanCombineCombinations) {
  std::unique_ptr<Parameter<int>> combined1{combine<int>(new DiscreteParameter<int>(1),
                                                         new DiscreteParameter<int>(2),
                                                         new RangedParameter<int>(3, 4))};
  std::unique_ptr<Parameter<int>> combined2{combine<int>(new DiscreteParameter<int>(5),
                                                         new DiscreteParameter<int>(6),
                                                         new RangedParameter<int>(7, 10))};
  std::unique_ptr<Parameter<int>> combined{
      combine<int>(std::move(combined1), std::move(combined2))};
  for (int i = 1; i <= 10; ++i) {
    EXPECT_TRUE(combined->is_allowed(i)) << "i: " << i;
  }
  EXPECT_FALSE(combined->is_allowed(0));
  EXPECT_FALSE(combined->is_allowed(11));
}

}  // namespace tvsc::control
