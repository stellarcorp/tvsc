#pragma once

#include <atomic>
#include <filesystem>
#include <future>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "SoapySDR/Device.hpp"

namespace tvsc::radio {

class DeviceGuard final {
 private:
  std::unordered_set<std::string> guarded_devices_{};

 public:
  bool is_guarded(std::string_view device_name) const {
    return guarded_devices_.count(std::string{device_name}) == 1;
  }

  /**
   * Operator acting as a filter to pass (return true) unguarded devices.
   */
  bool operator()(std::string_view device_name) const { return !is_guarded(device_name); }

  /**
   * Operator acting as a filter to pass unguarded devices. Guarded devices are removed from the
   * results.
   */
  SoapySDR::KwargsList operator()(SoapySDR::KwargsList in) const { return in; }

  bool guard(std::string_view device_name) { return guarded_devices_.emplace(device_name).second; }

  bool unguard(std::string_view device_name) {
    return guarded_devices_.erase(std::string{device_name}) == 1;
  }
};

class Soapy final {
 private:
  std::vector<SoapySDR::Device*> devices_{};
  DeviceGuard device_guard_{};

  std::atomic<bool> stop_server_{true};
  int server_result_cached_{};
  std::future<int> server_result_{};

 public:
  Soapy();
  ~Soapy();

  void start_server();
  void shutdown_server();
  int wait_on_server();

  std::vector<std::string> modules() const;

  /**
   * Load all modules under this path. Path should be a directory.
   */
  void load_modules(std::filesystem::path module_path, bool recursive = true);
  void unload_modules(std::filesystem::path module_path, bool recursive = true);

  /**
   * Load a single module. Path should point to a single filename.
   */
  void load_module(std::filesystem::path module_path);
  void unload_module(std::filesystem::path module_path);

  std::filesystem::path find_module_path(std::string_view module_name) const;
  bool contains_module(std::string_view module_name) const;

  std::string load_error_message(std::string_view module_name) const;
  bool loaded_successfully(const std::string_view module_name) const;

  std::string abi_version() const;
  std::string module_abi_version(std::string_view module_name) const;

  // Guard a device by making it no longer visible as a Soapy device. A device is guarded to allow
  // that particular device to be controlled outside of the SoapySDR ecosystem. An unguarded device
  // will be visible and can be controlled from SDR software, like CubicSDR. A guarded device will
  // not be visible from SDR software.
  //
  // For example, if you want to use a device to control the flight of a drone, that device should
  // be guarded so that someone cannot take control of that radio in SDR software and possibly cause
  // the drone to crash.
  void guard_device(std::string_view device_name) { device_guard_.guard(device_name); }
  void unguard_device(std::string_view device_name) { device_guard_.unguard(device_name); }
  std::vector<std::string> guarded_devices() const;
  std::vector<std::string> unguarded_devices() const;
  bool is_guarded(std::string_view device_name) const {
    return device_guard_.is_guarded(device_name);
  }

  std::vector<std::string> devices() const;
  bool has_device(std::string_view device_name) const;
  SoapySDR::Device& instantiate_device(std::string_view device_name);
};

}  // namespace tvsc::radio
