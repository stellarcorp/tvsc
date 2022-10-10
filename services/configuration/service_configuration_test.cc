#include "services/configuration/service_configuration.h"

#include "gtest/gtest.h"

namespace tvsc::service::configuration {

TEST(ServiceConfigurationTest, CanReadConfiguration) {
  ServiceConfiguration conf{ServiceConfiguration::load("services/configuration/virtual_tower_services.conf")};
  EXPECT_TRUE(conf.contains("echo"));
}

}  // namespace tvsc::service::configuration
