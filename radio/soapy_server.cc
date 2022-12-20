#include "radio/soapy_server.h"

#include <future>
#include <stdexcept>

#include "SoapySDR/Device.hpp"
#include "radio/soapy.h"
#include "soapy_server_runner.h"

namespace tvsc::radio {

SoapyServer::~SoapyServer() { shutdown(); }

void SoapyServer::start() {
  if (stop_server_ == false) {
    // We are already running the server.
    throw std::domain_error("Attempt to start soapy server when it is already running.");
  }

  stop_server_ = false;
  server_result_ = std::async(std::launch::async, run_soapy_server, std::ref(stop_server_),
                              soapy_->create_guarded_device_filter());
}

void SoapyServer::shutdown() {
  if (stop_server_ == false) {
    stop_server_ = true;
  }
}

int SoapyServer::wait() {
  server_result_.wait();
  server_result_cached_ = server_result_.get();
  return server_result_cached_;
}

}  // namespace tvsc::radio
