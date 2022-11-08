#pragma once

#include "SoapySDR/Device.hpp"
#include "SoapySDR/Types.hpp"

namespace tvsc::services::radio::server::modules {

// Find and make functions for the dummy receiver. These functions form the basis of the integration with SoapySDR.
SoapySDR::KwargsList find_dummy_receiver(const SoapySDR::Kwargs &args);
SoapySDR::Device *make_dummy_receiver(const SoapySDR::Kwargs &args);

// Entry point for the shared library when loaded via dlopen(3).
void create_registration() __attribute__((constructor));

}  // namespace tvsc::services::radio::server::modules
