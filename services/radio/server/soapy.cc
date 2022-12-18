#include "services/radio/server/soapy.h"

#include <atomic>
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

namespace tvsc::services::radio::server {

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

Soapy::Soapy() {
  putenv(SOAPY_SDR_PLUGIN_PATH_EXPR);
  // putenv(DLOPEN_BIND_NOW); // Force dlopen() to fully bind when the module is loaded. Useful for debugging link
  // issues.
  SoapySDR::registerLogHandler(log_to_glog);

  SoapySDR::logf(SOAPY_SDR_DEBUG, "%s:%d Soapy::Soapy() -- SoapySDR logging enabled.", __FILE__, __LINE__);

  LOG(INFO) << "Soapy ABI version: " << abi_version();

  SoapySDR::loadModules();
  if (modules().empty()) {
    LOG(ERROR) << "<No Soapy modules found>\n";
  } else {
    LOG(INFO) << "Loaded modules:\n";
    for (const auto& module : modules()) {
      LOG(INFO) << "\t" << module << "\n";
    }
  }

  SoapySDR::Device::enumerate();
}

Soapy::~Soapy() {
  shutdown_server();

  for (SoapySDR::Device* device : devices_) {
    SoapySDR::Device::unmake(device);
  }
}

void Soapy::start_server() {
  if (stop_server_ == false) {
    // We are already running the server.
    throw std::domain_error("Attempt to start soapy server when it is already running.");
  }

  stop_server_ = false;
  server_result_ = std::async(std::launch::async, run_soapy_server, std::ref(stop_server_));
}

void Soapy::shutdown_server() {
  if (stop_server_ == false) {
    stop_server_ = true;
  }
}

int Soapy::wait_on_server() {
  server_result_.wait();
  server_result_cached_ = server_result_.get();
  return server_result_cached_;
}

std::vector<std::string> Soapy::modules() const {
  std::vector<std::string> result{};
  for (const std::filesystem::path path : SoapySDR::listModules()) {
    result.emplace_back(path.filename().string());
  }
  return result;
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
    throw std::logic_error("Loader result has too many entries (expected 1; has " + to_string(args.size()) +
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
    throw std::logic_error("Loader result has too many entries (expected 1; has " + to_string(args.size()) +
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

}  // namespace tvsc::services::radio::server
