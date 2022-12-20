#pragma once

#include <atomic>
#include <future>

#include "radio/soapy.h"

namespace tvsc::radio {

class SoapyServer final {
 private:
  const Soapy* soapy_;

  std::atomic<bool> stop_server_{true};
  int server_result_cached_{};
  std::future<int> server_result_{};

 public:
  SoapyServer(const Soapy& soapy) : soapy_(&soapy) {}
  ~SoapyServer();

  void start();
  void shutdown();
  int wait();
};

}  // namespace tvsc::radio
