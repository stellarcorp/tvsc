#pragma once

#include <atomic>
#include <functional>

#include "SoapySDR/Device.hpp"
#include "gflags/gflags.h"

DECLARE_string(soapy_url);

// Note: this function does not return unless there is an error, or the server has been stopped.
int run_soapy_server(std::atomic<bool>& stop_server,
                     std::function<bool(const SoapySDR::Kwargs& device)> deviceFilter);
