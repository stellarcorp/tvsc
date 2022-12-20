#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "SoapySDR/Device.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySDR/Types.hpp"
#include "SoapySDR/Version.hpp"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include "radio/soapy.h"

namespace tvsc::services::radio::server {

const char DUMMY_RADIO_MODULE_NAME[]{"libdummy_radio.so"};
const char DUMMY_RADIO_DEVICE_NAME[]{"dummy_receiver"};

class ModuleTest : public ::testing::Test {
 public:
  std::unique_ptr<tvsc::radio::Soapy> soapy{};

  void SetUp() override { soapy.reset(new tvsc::radio::Soapy()); }

  void TearDown() override { soapy.reset(); }

  ::testing::AssertionResult contains_module(const std::string_view module_name) {
    if (soapy->contains_module(module_name)) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure()
             << "No module named '" << std::string{module_name} << "'";
    }
  }

  ::testing::AssertionResult load_success(const std::string_view module_name) {
    if (!soapy->loaded_successfully(module_name)) {
      return ::testing::AssertionFailure()
             << "Loader result not empty: " << soapy->load_error_message(module_name);
    }
    return ::testing::AssertionSuccess();
  }

  ::testing::AssertionResult has_correct_abi_version(const std::string_view module_name) {
    const std::string module_version{soapy->module_abi_version(module_name)};
    if (module_version == SoapySDR::getABIVersion()) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure() << "Incorrect module version: " << module_version;
    }
  }

  ::testing::AssertionResult has_find_function(const std::string_view device_name) {
    bool found{false};
    for (const auto& entry : SoapySDR::Registry::listFindFunctions()) {
      LOG(INFO) << "Find function -- entry.first: " << entry.first;
      if (entry.first == device_name and entry.second != nullptr) {
        found = true;
      }
    }

    if (found) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure() << "No find function for device '" << device_name << "'";
    }
  }

  ::testing::AssertionResult has_make_function(const std::string_view device_name) {
    bool found{false};
    for (const auto& entry : SoapySDR::Registry::listMakeFunctions()) {
      LOG(INFO) << "Make function -- entry.first: " << entry.first;
      if (entry.first == device_name and entry.second != nullptr) {
        found = true;
      }
    }

    if (found) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure() << "No make function for device '" << device_name << "'";
    }
  }

  ::testing::AssertionResult has_device(const std::string_view device_name) {
    if (!soapy->has_device(device_name)) {
      return ::testing::AssertionFailure() << "No such device found: " << device_name;
    }
    return ::testing::AssertionSuccess();
  }

  ::testing::AssertionResult can_create(const std::string_view device_name) {
    // instantiate_device() throws an exception if it is unsuccessful.
    [[maybe_unused]] SoapySDR::Device& device{soapy->instantiate_device(device_name)};
    return ::testing::AssertionSuccess();
  }

  ::testing::AssertionResult enumerate_contains(const std::string_view device_name) {
    bool found{false};
    for (const auto& device_args : SoapySDR::Device::enumerate()) {
      LOG(INFO) << "device_args: " << SoapySDR::KwargsToString(device_args);
      if (device_args.count("driver") == 1) {
        if (device_name == device_args.at("driver")) {
          found = true;
          break;
        }
      }
    }

    if (found) {
      return ::testing::AssertionSuccess();
    } else {
      return ::testing::AssertionFailure() << "No device named '" << device_name << "'";
    }
  }
};

TEST_F(ModuleTest, ModuleDirectoryConfiguration) {
  LOG(INFO) << "Working directory: " << std::filesystem::current_path().string();

  for (const auto& path : SoapySDR::listSearchPaths()) {
    LOG(INFO) << "Search path: " << path;
  }

  const auto modules{SoapySDR::listModules()};
  if (modules.empty()) {
    LOG(INFO) << "<No Soapy modules found>\n";
  } else {
    LOG(INFO) << "Successfully loaded modules:\n";
    for (const auto& module : modules) {
      LOG(INFO) << "\t" << module << "\n";
    }
  }

  EXPECT_FALSE(modules.empty());
}

TEST_F(ModuleTest, ContainsDummyRadio) { EXPECT_TRUE(contains_module(DUMMY_RADIO_MODULE_NAME)); }

TEST_F(ModuleTest, DummyRadioLoaderResultHasNoError) {
  EXPECT_TRUE(load_success(DUMMY_RADIO_MODULE_NAME));
}

TEST_F(ModuleTest, DummyRadioHasCorrectModuleVersion) {
  EXPECT_TRUE(has_correct_abi_version(DUMMY_RADIO_MODULE_NAME));
}

TEST_F(ModuleTest, DummyRadioHasFindFunction) {
  EXPECT_TRUE(has_find_function(DUMMY_RADIO_DEVICE_NAME));
}

TEST_F(ModuleTest, DummyRadioHasMakeFunction) {
  EXPECT_TRUE(has_make_function(DUMMY_RADIO_DEVICE_NAME));
}

TEST_F(ModuleTest, DummyRadioIsDevice) { EXPECT_TRUE(has_device(DUMMY_RADIO_DEVICE_NAME)); }

TEST_F(ModuleTest, EnumerateContainsDummyRadio) {
  EXPECT_TRUE(enumerate_contains(DUMMY_RADIO_DEVICE_NAME));
}

TEST_F(ModuleTest, CanCreateDummyRadio) { EXPECT_TRUE(can_create(DUMMY_RADIO_DEVICE_NAME)); }

}  // namespace tvsc::services::radio::server
