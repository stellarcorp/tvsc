#include <string>

#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySDR/Version.h"
#include "radio/dummy_receiver.h"

namespace tvsc::services::radio::server::modules {

// Entry point for the shared library when loaded via dlopen(3).
void create_registration() __attribute__((constructor));
void create_registration() { tvsc::radio::register_dummy_receiver(); }

}  // namespace tvsc::services::radio::server::modules
