#include "SoapySDR/Device.hpp"
#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"

/***********************************************************************
 * Device interface
 **********************************************************************/
class DummyRadioDevice final : public SoapySDR::Device {};

/***********************************************************************
 * Find available devices
 **********************************************************************/
SoapySDR::KwargsList find_dummy_radio(const SoapySDR::Kwargs &args) {
  SoapySDR::log(SOAPY_SDR_DEBUG, "find_dummy_radio() -- args:");
  for (const auto &arg : args) {
    SoapySDR::logf(SOAPY_SDR_DEBUG, "\targ.first: %s, arg.second: %s", arg.first, arg.second);
  }
  return SoapySDR::KwargsList{args};
}

/***********************************************************************
 * Make device instance
 **********************************************************************/
SoapySDR::Device *make_dummy_radio(const SoapySDR::Kwargs &args) {
  SoapySDR::log(SOAPY_SDR_DEBUG, "make_dummy_radio() -- args:");
  for (const auto &arg : args) {
    SoapySDR::logf(SOAPY_SDR_DEBUG, "\targ.first: %s, arg.second: %s", arg.first, arg.second);
  }
  return new DummyRadioDevice{};
}

/***********************************************************************
 * Registration
 **********************************************************************/
/***********************************************************************
 * Module loader shared data structures
 **********************************************************************/
std::string &getModuleLoading(void);

std::map<std::string, SoapySDR::Kwargs> &getLoaderResults(void);

void create_registration() __attribute__((constructor));
void create_registration() {
  static SoapySDR::Registry module_registration{"dummy_radio", &find_dummy_radio, &make_dummy_radio,
                                                SOAPY_SDR_ABI_VERSION};
  static SoapySDR::ModuleVersion module_version("0.8-2");
  const std::string &current_module{getModuleLoading()};
  if (current_module.empty()) {
    SoapySDR::log(SOAPY_SDR_FATAL, "create_registration() -- current_module is empty during module registration");
    abort();
  } else {
    SoapySDR::logf(SOAPY_SDR_TRACE, "create_registration() -- current_module: %s", current_module);
  }
}
