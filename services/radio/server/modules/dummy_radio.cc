#include "services/radio/server/modules/dummy_radio.h"

#include <string>

#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySDR/Version.h"

std::string &getModuleLoading(void);

namespace tvsc::services::radio::server::modules {

void create_registration() {
  static SoapySDR::Registry module_registration{"dummy_receiver", &find_dummy_receiver, &make_dummy_receiver,
                                                SOAPY_SDR_ABI_VERSION};
  static SoapySDR::ModuleVersion module_version{SOAPY_SDR_ABI_VERSION};

  // If the SoapySDR library does not have the correct linkage, we get a situation where the registration code has
  // multiple copies of the static variables, such as the current module being loaded. This block of code identifies
  // that scenario and forces an immediate failure with a log message so that the build issue can be more easily
  // identified.
  const std::string &current_module{getModuleLoading()};
  if (current_module.empty()) {
    SoapySDR::log(SOAPY_SDR_FATAL, "create_registration() -- current_module is empty during module registration");
    abort();
  } else {
    SoapySDR::logf(SOAPY_SDR_TRACE, "create_registration() -- current_module: %s", current_module);
  }
}

}  // namespace tvsc::services::radio::server::modules
