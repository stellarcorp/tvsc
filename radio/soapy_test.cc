#include "radio/soapy.h"

#include <filesystem>
#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"

namespace tvsc::radio {

const char DUMMY_RECEIVER_MODULE_PATH[]{"radio/libdummy_radio_module.so"};
const char DUMMY_RECEIVER_DEVICE_NAME[]{"dummy_receiver"};

std::string working_directory() {
  std::filesystem::path path{std::filesystem::current_path()};
  return path.string();
}

class SoapyDeviceGuardTest : public ::testing::Test {
 public:
  Soapy soapy{};

  void SetUp() override { soapy.load_module(DUMMY_RECEIVER_MODULE_PATH); }

  void TearDown() override { soapy.unload_module(DUMMY_RECEIVER_MODULE_PATH); }
};

TEST_F(SoapyDeviceGuardTest, HasDummyReceiver) {
  EXPECT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME))
      << "Working directory: '" << working_directory() << "'";
}

TEST_F(SoapyDeviceGuardTest, CanUnloadModule) {
  ASSERT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME))
      << "Working directory: '" << working_directory() << "'";

  soapy.unload_module(DUMMY_RECEIVER_MODULE_PATH);
  EXPECT_FALSE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME));

  soapy.load_module(DUMMY_RECEIVER_MODULE_PATH);
  EXPECT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME));
}

TEST_F(SoapyDeviceGuardTest, CanGuardDevice) {
  ASSERT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME))
      << "Working directory: '" << working_directory() << "'";
  ASSERT_FALSE(soapy.is_guarded(DUMMY_RECEIVER_DEVICE_NAME));

  soapy.guard_device(DUMMY_RECEIVER_DEVICE_NAME);
  EXPECT_TRUE(soapy.is_guarded(DUMMY_RECEIVER_DEVICE_NAME));
  EXPECT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME));

  soapy.unguard_device(DUMMY_RECEIVER_DEVICE_NAME);
  EXPECT_FALSE(soapy.is_guarded(DUMMY_RECEIVER_DEVICE_NAME));
  EXPECT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME));
}

}  // namespace tvsc::radio
