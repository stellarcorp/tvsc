#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

#include "SoapySDR/Device.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySDR/Types.hpp"
#include "SoapySDR/Version.hpp"
#include "glog/logging.h"
#include "gtest/gtest.h"

char SOAPY_SDR_PLUGIN_PATH_EXPR[]{"SOAPY_SDR_PLUGIN_PATH=services/radio/server/modules"};
const char DUMMY_RADIO_MODULE_NAME[]{"libdummy_radio.so"};
const char RTLSDR_MODULE_NAME[]{"libsoapy_rtlsdr.so"};
const char DUMMY_RADIO_DEVICE_NAME[]{"dummy_radio"};
const char RTLSDR_DEVICE_NAME[]{"rtlsdr"};

using DevicePtr = std::unique_ptr<SoapySDR::Device, void (*)(SoapySDR::Device*)>;
DevicePtr create_device(const std::string_view device_name) {
  SoapySDR::Kwargs args{};
  args["driver"] = device_name;

  SoapySDR::Device* device{SoapySDR::Device::make(args)};
  return DevicePtr(device, SoapySDR::Device::unmake);
}

std::string find_module_path(const std::string_view module_name) {
  bool found{false};
  std::string module_path{};
  for (const std::filesystem::path path : SoapySDR::listModules()) {
    if (path.filename().string() == module_name) {
      found = true;
      module_path = path.string();
    }
  }
  if (found) {
    return module_path;
  } else {
    throw std::domain_error("No module named '" + std::string{module_name} + "'");
  }
}

::testing::AssertionResult contains_module(const std::string_view module_name) {
  bool found{false};
  std::string module_path{};
  for (const std::filesystem::path path : SoapySDR::listModules()) {
    if (path.filename().string() == module_name) {
      found = true;
      module_path = path.string();
    }
  }
  if (found) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "No module named '" << std::string{module_name} << "'";
  }
}

::testing::AssertionResult load_success(const std::string_view module_name) {
  std::string module_path{find_module_path(module_name)};

  const SoapySDR::Kwargs args{SoapySDR::getLoaderResult(module_path)};

  if (args.size() != 1) {
    return ::testing::AssertionFailure() << "Loader result has too many entries (expected 1; has " << args.size()
                                         << "). Format not understood: " << SoapySDR::KwargsToString(args);
  }

  for (const auto& entry : args) {
    if (!entry.second.empty()) {
      return ::testing::AssertionFailure() << "Loader result not empty: " << SoapySDR::KwargsToString(args);
    }
  }
  return ::testing::AssertionSuccess();
}

::testing::AssertionResult has_correct_abi_version(const std::string_view module_name) {
  std::string module_path{find_module_path(module_name)};

  const std::string module_version{SoapySDR::getModuleVersion(module_path)};

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

::testing::AssertionResult can_create(const std::string_view device_name) {
  DevicePtr device{create_device(device_name)};
  if (device) {
    return ::testing::AssertionSuccess();
  } else {
    return ::testing::AssertionFailure() << "Could not create device '" << device_name << "'";
  }
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

class ModuleTest : public ::testing::Test {
 public:
  void SetUp() override {
    putenv(SOAPY_SDR_PLUGIN_PATH_EXPR);
    SoapySDR::loadModules();
  }

  void TearDown() override { SoapySDR::unloadModules(); }
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
TEST_F(ModuleTest, ContainsRtlSdr) { EXPECT_TRUE(contains_module(RTLSDR_MODULE_NAME)); }

TEST_F(ModuleTest, DummyRadioLoaderResultHasNoError) { EXPECT_TRUE(load_success(DUMMY_RADIO_MODULE_NAME)); }
TEST_F(ModuleTest, RtlSdrLoaderResultHasNoError) { EXPECT_TRUE(load_success(RTLSDR_MODULE_NAME)); }

TEST_F(ModuleTest, DummyRadioHasCorrectModuleVersion) { EXPECT_TRUE(has_correct_abi_version(DUMMY_RADIO_MODULE_NAME)); }
TEST_F(ModuleTest, RtlSdrHasCorrectModuleVersion) { EXPECT_TRUE(has_correct_abi_version(RTLSDR_MODULE_NAME)); }

TEST_F(ModuleTest, DummyRadioHasFindFunction) { EXPECT_TRUE(has_find_function(DUMMY_RADIO_DEVICE_NAME)); }
TEST_F(ModuleTest, RtlSdrHasFindFunction) { EXPECT_TRUE(has_find_function(RTLSDR_DEVICE_NAME)); }

TEST_F(ModuleTest, DummyRadioHasMakeFunction) { EXPECT_TRUE(has_make_function(DUMMY_RADIO_DEVICE_NAME)); }
TEST_F(ModuleTest, RtlSdrHasMakeFunction) { EXPECT_TRUE(has_make_function(RTLSDR_DEVICE_NAME)); }

TEST_F(ModuleTest, EnumerateContainsDummyRadio) { EXPECT_TRUE(enumerate_contains(DUMMY_RADIO_DEVICE_NAME)); }
TEST_F(ModuleTest, EnumerateContainsRtlSdr) { EXPECT_TRUE(enumerate_contains(RTLSDR_DEVICE_NAME)); }

TEST_F(ModuleTest, CanCreateDummyRadio) { EXPECT_TRUE(can_create(DUMMY_RADIO_DEVICE_NAME)); }
TEST_F(ModuleTest, CanCreateRtlSdr) { EXPECT_TRUE(can_create(RTLSDR_DEVICE_NAME)); }
