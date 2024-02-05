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

}  // namespace tvsc::control
