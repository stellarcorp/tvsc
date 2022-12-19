#include "radio/soapy.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "gmock/gmock.h"

namespace tvsc::radio {

using ::testing::ContainerEq;

const char DUMMY_RECEIVER_MODULE_PATH[]{"radio/libdummy_radio_module.so"};
const char DUMMY_RECEIVER_DEVICE_NAME[]{"dummy_receiver"};

std::string working_directory() {
  std::filesystem::path path{std::filesystem::current_path()};
  return path.string();
}

TEST(DeviceGuardTest, CanUseAsUnaryPredicate) {
  const std::string REMOVE{"remove"};
  DeviceGuard filter{};
  filter.guard(REMOVE);

  std::vector<std::string> kept_things{};
  std::vector<std::string> things{};
  things.emplace_back("keep1");
  kept_things.emplace_back("keep1");
  things.emplace_back(REMOVE);
  things.emplace_back("keep2");
  kept_things.emplace_back("keep2");

  things.erase(std::remove_if(things.begin(), things.end(), filter));

  EXPECT_THAT(things, ContainerEq(kept_things));
}

class SoapyTest : public ::testing::Test {
 public:
  Soapy soapy{};

  void SetUp() override { soapy.load_module(DUMMY_RECEIVER_MODULE_PATH); }

  void TearDown() override { soapy.unload_module(DUMMY_RECEIVER_MODULE_PATH); }
};

TEST_F(SoapyTest, HasDummyReceiver) {
  EXPECT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME))
      << "Working directory: '" << working_directory() << "'";
}

TEST_F(SoapyTest, CanUnloadModule) {
  ASSERT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME))
      << "Working directory: '" << working_directory() << "'";

  soapy.unload_module(DUMMY_RECEIVER_MODULE_PATH);
  EXPECT_FALSE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME));

  soapy.load_module(DUMMY_RECEIVER_MODULE_PATH);
  EXPECT_TRUE(soapy.has_device(DUMMY_RECEIVER_DEVICE_NAME));
}

TEST_F(SoapyTest, CanGuardDevice) {
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
