#include "SoapySDR/Device.hpp"
#include "SoapySDR/Registry.hpp"
#include "glog/logging.h"

/***********************************************************************
 * Device interface
 **********************************************************************/
class DummyRadioDevice final : public SoapySDR::Device {};

/***********************************************************************
 * Find available devices
 **********************************************************************/
SoapySDR::KwargsList findMyDevice(const SoapySDR::Kwargs &args) {
  LOG(INFO) << "args: ";
  for (const auto &arg : args) {
    LOG(INFO) << "arg.first: " << arg.first << ", arg.second: " << arg.second;
  }
  return SoapySDR::KwargsList{args};
}

/***********************************************************************
 * Make device instance
 **********************************************************************/
SoapySDR::Device *makeMyDevice(const SoapySDR::Kwargs &args) {
  LOG(INFO) << "args: ";
  for (const auto &arg : args) {
    LOG(INFO) << "arg.first: " << arg.first << ", arg.second: " << arg.second;
  }
  return new DummyRadioDevice{};
}

/***********************************************************************
 * Registration
 **********************************************************************/
static SoapySDR::Registry module_registration{"dummy_radio", &findMyDevice, &makeMyDevice, SOAPY_SDR_ABI_VERSION};
