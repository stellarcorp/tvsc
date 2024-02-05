#include "control/parameter_domain.h"
#include "gtest/gtest.h"

namespace tvsc::control {

TEST(CategoricalParameterDomainTest, CanDetectAllowedValue) {
  auto domain = configure_categorical_domain<int>().with_values(1, 3).create();
  EXPECT_TRUE(domain->in_domain(1));
  EXPECT_TRUE(domain->in_domain(3));
  EXPECT_FALSE(domain->in_domain(0));
  EXPECT_FALSE(domain->in_domain(2));
  EXPECT_FALSE(domain->in_domain(4));
}

TEST(CategoricalParameterDomainTest, CanAddValuesMultipleTimes) {
  auto domain = configure_categorical_domain<int>().with_values(1, 3).with_values(5, 6, 7, 8).create();
  EXPECT_TRUE(domain->in_domain(1));
  EXPECT_TRUE(domain->in_domain(3));
  EXPECT_TRUE(domain->in_domain(5));
  EXPECT_TRUE(domain->in_domain(6));
  EXPECT_TRUE(domain->in_domain(7));
  EXPECT_TRUE(domain->in_domain(8));
  EXPECT_FALSE(domain->in_domain(0));
  EXPECT_FALSE(domain->in_domain(2));
  EXPECT_FALSE(domain->in_domain(4));
}

TEST(CategoricalParameterDomainTest, SizeIsNumberOfValues) {
  auto domain = configure_categorical_domain<int>().with_values(1, 3).create();
  EXPECT_EQ(2, domain->size());
}

}  // namespace tvsc::control
