#include "services/configuration/service_location.h"

#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "services/configuration/service_configuration.h"
#include "services/configuration/service_descriptor.pb.h"

namespace tvsc::service::configuration {

ServiceDescriptor as_descriptor(std::string_view name, std::string_view end_point) {
  ServiceDescriptor result{};
  result.set_name(std::string{name});
  result.set_end_point(std::string{end_point});
  return result;
}

TEST(ServiceLocationTest, CanGetAddrFromConfiguration) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char CONFIGURED_BIND_ADDR[] = "hostname:2345";
  constexpr char OVERRIDE_BIND_ADDR[] = "";

  ServiceConfiguration configuration{};
  configuration.add(as_descriptor("service1", "hostname:1234"));
  configuration.add(as_descriptor(SERVICE_NAME, CONFIGURED_BIND_ADDR));
  configuration.add(as_descriptor("service3", "hostname:3456"));

  const std::string end_point{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(CONFIGURED_BIND_ADDR, end_point);
}

TEST(ServiceLocationTest, CanOverrideConfiguration) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char CONFIGURED_BIND_ADDR[] = "hostname:2345";
  constexpr char OVERRIDE_BIND_ADDR[] = "hostname:12345";

  ServiceConfiguration configuration{};
  configuration.add(as_descriptor("service1", "hostname:1234"));
  configuration.add(as_descriptor(SERVICE_NAME, CONFIGURED_BIND_ADDR));
  configuration.add(as_descriptor("service3", "hostname:3456"));

  const std::string end_point{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(OVERRIDE_BIND_ADDR, end_point);
}

TEST(ServiceLocationTest, CanOverrideDefault) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char OVERRIDE_BIND_ADDR[] = "hostname:12345";

  ServiceConfiguration configuration{};
  configuration.add(as_descriptor("service1", "hostname:1234"));
  configuration.add(as_descriptor("service3", "hostname:3456"));

  const std::string end_point{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(OVERRIDE_BIND_ADDR, end_point);
}

TEST(ServiceLocationTest, UsesDefaultIfNotConfigured) {
  constexpr char SERVICE_NAME[] = "service2";
  constexpr char DEFAULT_BIND_ADDR[] = "some_other_host:7890";
  constexpr char OVERRIDE_BIND_ADDR[] = "";

  ServiceConfiguration configuration{};
  configuration.add(as_descriptor("service1", "hostname:1234"));
  configuration.add(as_descriptor("service3", "hostname:3456"));

  const std::string end_point{
      determine_service_addr(SERVICE_NAME, OVERRIDE_BIND_ADDR, configuration, DEFAULT_BIND_ADDR)};
  EXPECT_EQ(DEFAULT_BIND_ADDR, end_point);
}

}  // namespace tvsc::service::configuration
