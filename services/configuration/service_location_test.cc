#include "services/configuration/service_location.h"

#include "gtest/gtest.h"
#include "services/configuration/service_configuration.h"

namespace tvsc::service::configuration {

TEST(ServiceLocationTest, CanGetAddrFromConfiguration) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char CONFIGURED_BIND_ADDR[] = "hostname:2345";
  constexpr char OVERRIDE_BIND_ADDR[] = "";

  ServiceConfiguration configuration{};
  configuration.insert("service1", "hostname:1234");
  configuration.insert(SERVICE_NAME, CONFIGURED_BIND_ADDR);
  configuration.insert("service3", "hostname:3456");

  const std::string bind_addr{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(CONFIGURED_BIND_ADDR, bind_addr);
}

TEST(ServiceLocationTest, CanOverrideConfiguration) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char CONFIGURED_BIND_ADDR[] = "hostname:2345";
  constexpr char OVERRIDE_BIND_ADDR[] = "hostname:12345";

  ServiceConfiguration configuration{};
  configuration.insert("service1", "hostname:1234");
  configuration.insert(SERVICE_NAME, CONFIGURED_BIND_ADDR);
  configuration.insert("service3", "hostname:3456");

  const std::string bind_addr{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(OVERRIDE_BIND_ADDR, bind_addr);
}

TEST(ServiceLocationTest, CanOverrideDefault) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char OVERRIDE_BIND_ADDR[] = "hostname:12345";

  ServiceConfiguration configuration{};
  configuration.insert("service1", "hostname:1234");
  configuration.insert("service3", "hostname:3456");

  const std::string bind_addr{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(OVERRIDE_BIND_ADDR, bind_addr);
}

TEST(ServiceLocationTest, UsesDefaultIfNotConfigured) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char OVERRIDE_BIND_ADDR[] = "";

  ServiceConfiguration configuration{};
  configuration.insert("service1", "hostname:1234");
  configuration.insert("service3", "hostname:3456");

  const std::string bind_addr{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(DEFAULT_BIND_ADDR, bind_addr);
}

}  // namespace tvsc::service::configuration
