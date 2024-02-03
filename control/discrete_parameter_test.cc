#include "control/discrete_parameter.h"

#include "gtest/gtest.h"

namespace tvsc::control {

TEST(DiscreteParameterTest, CanDetectAllowedValue) {
  DiscreteParameter<int> p{1};
  EXPECT_TRUE(p.is_allowed(1));
  EXPECT_FALSE(p.is_allowed(0));
}

TEST(DiscreteParameterTest, CanCopy) {
  DiscreteParameter<int> p1{1};
  DiscreteParameter<int> p2{p1};
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(DiscreteParameterTest, CanMoveConstruct) {
  DiscreteParameter<int> p1{1};
  DiscreteParameter<int> p2{std::move(p1)};
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(DiscreteParameterTest, CanAssign) {
  DiscreteParameter<int> p1{1};
  DiscreteParameter<int> p2{0};

  p2 = p1;

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(DiscreteParameterTest, CanMoveAssign) {
  DiscreteParameter<int> p1{1};
  DiscreteParameter<int> p2{0};

  p2 = std::move(p1);

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

}  // namespace tvsc::control
