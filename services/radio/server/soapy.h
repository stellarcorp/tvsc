#pragma once

#include <atomic>
#include <filesystem>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "SoapySDR/Device.hpp"

namespace tvsc::services::radio::server {

class Soapy final {
 private:
  std::vector<SoapySDR::Device*> devices_{};
  std::atomic<bool> stop_server_{false};
  std::thread server_thread_{};

 public:
  Soapy();
  ~Soapy();

  void start_server();
  void shutdown_server();
  void wait_on_server();

  std::vector<std::string> modules() const;

  std::filesystem::path find_module_path(const std::string_view module_name) const;
  bool contains_module(const std::string_view module_name) const;

  std::string load_error_message(const std::string_view module_name) const;
  bool loaded_successfully(const std::string_view module_name) const;

  std::string abi_version(const std::string_view module_name) const;

  std::vector<std::string> devices() const;
  bool has_device(const std::string_view device_name) const;
  SoapySDR::Device& instantiate_device(const std::string_view device_name);
};

}  // namespace tvsc::services::radio::server
