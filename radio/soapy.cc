#include "radio/soapy.h"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <functional>
#include <future>
#include <string>
#include <string_view>

#include "SoapySDR/Device.hpp"
#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySDR/Types.hpp"
#include "SoapySDR/Version.hpp"
#include "glog/logging.h"
#include "soapy_server.h"

namespace tvsc::radio {

char SOAPY_SDR_PLUGIN_PATH_EXPR[]{"SOAPY_SDR_PLUGIN_PATH=services/radio/server/modules"};
// char DLOPEN_BIND_NOW[]{"LD_BIND_NOW=true"};

inline void log_to_glog(const SoapySDRLogLevel logLevel, const char* message) {
  switch (logLevel) {
    case SOAPY_SDR_FATAL:
      LOG(FATAL) << message;
      break;
    case SOAPY_SDR_CRITICAL:
    case SOAPY_SDR_ERROR:
      LOG(ERROR) << message;
      break;
    case SOAPY_SDR_WARNING:
      LOG(WARNING) << message;
      break;
    case SOAPY_SDR_NOTICE:
    case SOAPY_SDR_INFO:
      LOG(INFO) << message;
      break;
    case SOAPY_SDR_DEBUG:
    case SOAPY_SDR_TRACE:
    case SOAPY_SDR_SSI:
      LOG(INFO) << message;
      break;
  }
}

void log_modules(const Soapy& soapy) {
  const auto modules{soapy.modules()};
  if (modules.empty()) {
    LOG(ERROR) << "<No Soapy modules found>\n";
  } else {
    LOG(INFO) << "Loaded modules:\n";
    for (const auto& module : modules) {
      LOG(INFO) << "\t" << module << "\n";
    }
  }
}

Soapy::Soapy() {
  putenv(SOAPY_SDR_PLUGIN_PATH_EXPR);
  // putenv(DLOPEN_BIND_NOW); // Force dlopen() to fully bind when the module is loaded. Useful for
  // debugging link issues.
  SoapySDR::registerLogHandler(log_to_glog);

  SoapySDR::logf(SOAPY_SDR_DEBUG, "%s:%d Soapy::Soapy() -- SoapySDR logging enabled.", __FILE__,
                 __LINE__);

  LOG(INFO) << "Soapy ABI version: " << abi_version();

  SoapySDR::loadModules();
  log_modules(*this);
  SoapySDR::Device::enumerate();
}

Soapy::~Soapy() {
  for (SoapySDR::Device* device : devices_) {
    SoapySDR::Device::unmake(device);
  }
}

std::vector<std::string> Soapy::modules() const {
  std::vector<std::string> result{};
  for (const std::filesystem::path path : SoapySDR::listModules()) {
    result.emplace_back(path.filename().string());
  }
  return result;
}

void Soapy::load_module(std::filesystem::path path) {
  const auto result = SoapySDR::loadModule(path.string());
  LOG(INFO) << "Loaded module at path '" << path.string() << "'. Result: " << result;
  log_modules(*this);
}

void Soapy::unload_module(std::filesystem::path path) {
  SoapySDR::unloadModule(path.string());
  log_modules(*this);
}

std::filesystem::path Soapy::find_module_path(const std::string_view module_name) const {
  for (const std::filesystem::path path : SoapySDR::listModules()) {
    if (path.filename().string() == module_name) {
      return path;
    }
  }
  throw std::domain_error("No module named '" + std::string{module_name} + "'");
}

bool Soapy::contains_module(const std::string_view module_name) const {
  for (const std::filesystem::path path : SoapySDR::listModules()) {
    if (path.filename().string() == module_name) {
      return true;
    }
  }
  return false;
}

std::string Soapy::load_error_message(const std::string_view module_name) const {
  using std::to_string;

  const std::string module_path{find_module_path(module_name)};
  const SoapySDR::Kwargs args{SoapySDR::getLoaderResult(module_path)};

  if (args.size() > 1) {
    throw std::logic_error("Loader result has too many entries (expected 1; has " +
                           to_string(args.size()) +
                           "). Format not understood: " + SoapySDR::KwargsToString(args));
  }

  if (args.size() == 0) {
    throw std::logic_error("Unknown load error. Empty loader result.");
  }

  return args.begin()->second;
}

bool Soapy::loaded_successfully(const std::string_view module_name) const {
  using std::to_string;

  const std::string module_path{find_module_path(module_name)};
  const SoapySDR::Kwargs args{SoapySDR::getLoaderResult(module_path)};

  if (args.size() > 1) {
    throw std::logic_error("Loader result has too many entries (expected 1; has " +
                           to_string(args.size()) +
                           "). Format not understood: " + SoapySDR::KwargsToString(args));
  }

  if (args.size() == 0) {
    return false;
  }

  return args.begin()->second.empty();
}

std::string Soapy::abi_version() const { return SoapySDR::getABIVersion(); }

std::string Soapy::module_abi_version(const std::string_view module_name) const {
  std::string module_path{find_module_path(module_name)};
  return SoapySDR::getModuleVersion(module_path);
}

std::vector<std::string> Soapy::devices() const {
  std::vector<std::string> result{};
  for (const auto& device_args : SoapySDR::Device::enumerate()) {
    LOG(INFO) << "device_args: " << SoapySDR::KwargsToString(device_args);
    if (device_args.count("driver") == 1) {
      result.emplace_back(device_args.at("driver"));
    }
  }
  return result;
}

std::vector<std::string> Soapy::guarded_devices() const {
  SoapySDR::KwargsList raw_devices{SoapySDR::Device::enumerate()};
  // Remove all devices that are *not* guarded.
  raw_devices.erase(std::remove_if(
      raw_devices.begin(), raw_devices.end(),
      [this](const SoapySDR::Kwargs& device) { return !device_guard_.is_guarded(device); }));

  std::vector<std::string> result{};
  for (const auto& device_args : raw_devices) {
    if (device_args.count("driver") == 1) {
      result.emplace_back(device_args.at("driver"));
    }
  }
  return result;
}

std::vector<std::string> Soapy::unguarded_devices() const {
  SoapySDR::KwargsList raw_devices{SoapySDR::Device::enumerate()};
  // Remove any device that is guarded.
  raw_devices.erase(std::remove_if(raw_devices.begin(), raw_devices.end(), device_guard_));

  std::vector<std::string> result{};
  for (const auto& device_args : raw_devices) {
    if (device_args.count("driver") == 1) {
      result.emplace_back(device_args.at("driver"));
    }
  }
  return result;
}

bool Soapy::has_device(const std::string_view device_name) const {
  for (const auto& entry : SoapySDR::Registry::listMakeFunctions()) {
    if (entry.first == device_name and entry.second != nullptr) {
      return true;
    }
  }
  return false;
}

SoapySDR::Device& Soapy::instantiate_device(const std::string_view device_name) {
  SoapySDR::Kwargs args{};
  args["driver"] = device_name;

  SoapySDR::Device* device{SoapySDR::Device::make(args)};
  if (device == nullptr) {
    throw std::logic_error("Could not instantiate device with name " + std::string{device_name});
  }
  devices_.push_back(device);
  return *device;
}

}  // namespace tvsc::radio
