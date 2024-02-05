#include "control/parameter_domain.h"
#include "gtest/gtest.h"

namespace tvsc::control {

TEST(ContinuousParameterDomainTest, CanDetectAllowedValue) {
  auto domain = configure_continuous_domain<int>().with_range(1, 3).create();
  EXPECT_TRUE(domain->in_domain(1));
  EXPECT_TRUE(domain->in_domain(2));
  EXPECT_TRUE(domain->in_domain(3));
  EXPECT_FALSE(domain->in_domain(0));
  EXPECT_FALSE(domain->in_domain(4));
}

TEST(ContinuousParameterDomainTest, CannotAddMultipleRanges) {
  auto domain = configure_continuous_domain<int>().with_range(5, 8).with_range(1, 3).create();
  EXPECT_TRUE(domain->in_domain(1));
  EXPECT_TRUE(domain->in_domain(2));
  EXPECT_TRUE(domain->in_domain(3));
  EXPECT_FALSE(domain->in_domain(5));
  EXPECT_FALSE(domain->in_domain(6));
  EXPECT_FALSE(domain->in_domain(7));
  EXPECT_FALSE(domain->in_domain(8));
  EXPECT_FALSE(domain->in_domain(0));
  EXPECT_FALSE(domain->in_domain(4));
}

TEST(ContinuousParameterDomainTest, CanDetectAllowedValuesInFloatRange) {
  auto domain = configure_continuous_domain<float>().with_range(1.f, 3.f).create();
  EXPECT_TRUE(domain->in_domain(1.f));
  EXPECT_TRUE(domain->in_domain(2.5f));
  EXPECT_TRUE(domain->in_domain(3.f));
  EXPECT_FALSE(domain->in_domain(3.1f));
  EXPECT_FALSE(domain->in_domain(4.f));
}

TEST(ContinuousParameterDomainTest, CanDetectAllowedValuesInFloatRangeWithExcludedEndPoints) {
  auto domain = configure_continuous_domain<float>().with_range(1.f, 3.f).exclude_low().exclude_high().create();
  EXPECT_FALSE(domain->in_domain(1.f));
  EXPECT_TRUE(domain->in_domain(1.5f));
  EXPECT_TRUE(domain->in_domain(2.5f));
  EXPECT_TRUE(domain->in_domain(2.9f));
  EXPECT_FALSE(domain->in_domain(3.f));
  EXPECT_FALSE(domain->in_domain(3.1f));
  EXPECT_FALSE(domain->in_domain(4.f));
}

TEST(ContinuousParameterDomainTest, SizeOfIntegralRangeIsNumberPossibleValues) {
  auto domain = configure_continuous_domain<int>().with_range(1, 3).create();
  EXPECT_EQ(3, domain->size());
  domain = configure_continuous_domain<int>().with_range(1, 3).exclude_low().create();
  EXPECT_EQ(2, domain->size());
  domain = configure_continuous_domain<int>().with_range(1, 3).exclude_high().create();
  EXPECT_EQ(2, domain->size());
  domain = configure_continuous_domain<int>().with_range(1, 3).exclude_low().exclude_high().create();
  EXPECT_EQ(1, domain->size());
}

}  // namespace tvsc::control
