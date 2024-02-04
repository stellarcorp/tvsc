#include "control/discrete_parameter_domain.h"

#include "gtest/gtest.h"

namespace tvsc::control {

TEST(DiscreteParameterDomainTest, CanDetectAllowedValue) {
  DiscreteParameterDomain<int> p{1};
  EXPECT_TRUE(p.is_allowed(1));
  EXPECT_FALSE(p.is_allowed(0));
}

TEST(DiscreteParameterDomainTest, CanCopy) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{p1};
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(DiscreteParameterDomainTest, CanMoveConstruct) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{std::move(p1)};
  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(DiscreteParameterDomainTest, CanAssign) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{0};

  p2 = p1;

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

TEST(DiscreteParameterDomainTest, CanMoveAssign) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{0};

  p2 = std::move(p1);

  EXPECT_TRUE(p2.is_allowed(1));
  EXPECT_FALSE(p2.is_allowed(0));
}

}  // namespace tvsc::control
