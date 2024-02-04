#include "control/discrete_parameter_domain.h"

#include "gtest/gtest.h"

namespace tvsc::control {

TEST(DiscreteParameterDomainTest, CanDetectAllowedValue) {
  DiscreteParameterDomain<int> p{1};
  EXPECT_TRUE(p.in_domain(1));
  EXPECT_FALSE(p.in_domain(0));
}

TEST(DiscreteParameterDomainTest, CanCopy) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{p1};
  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(0));
}

TEST(DiscreteParameterDomainTest, CanMoveConstruct) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{std::move(p1)};
  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(0));
}

TEST(DiscreteParameterDomainTest, CanAssign) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{0};

  p2 = p1;

  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(0));
}

TEST(DiscreteParameterDomainTest, CanMoveAssign) {
  DiscreteParameterDomain<int> p1{1};
  DiscreteParameterDomain<int> p2{0};

  p2 = std::move(p1);

  EXPECT_TRUE(p2.in_domain(1));
  EXPECT_FALSE(p2.in_domain(0));
}

TEST(DiscreteParameterDomainTest, SizeIsOne) {
  DiscreteParameterDomain<int> p1{1};
  EXPECT_EQ(1, p1.size());
}

}  // namespace tvsc::control
