#include <string>
#include <vector>

#include "SoapySDR/Constants.h"
#include "SoapySDR/Device.hpp"
#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"

constexpr double GAIN_MIN{0.};
constexpr double GAIN_MAX{1.};

/***********************************************************************
 * Device interface
 **********************************************************************/
class DummyRadioDevice final : public SoapySDR::Device {
 public:
  std::string getDriverKey() const override { return "dummy_radio"; }

  std::string getHardwareKey() const override { return "TVSC Dummy Radio"; }

  SoapySDR::Kwargs getHardwareInfo() const override {
    SoapySDR::Kwargs args{};

    args["origin"] = "https://stellarcorp.tv/";
    args["index"] = "0";
    args["label"] = "TVSC Dummy Radio";

    return args;
  }

  /*******************************************************************
   * Channels API
   ******************************************************************/
  size_t getNumChannels(int direction) const override {
    if (direction == SOAPY_SDR_TX) {
      return 0;
    } else if (direction == SOAPY_SDR_RX) {
      return 1;
    } else {
      return -1;
    }
  }

  bool getFullDuplex(int direction, size_t channel) const override { return false; }

  /*******************************************************************
   * Antenna API
   ******************************************************************/

  std::vector<std::string> listAntennas(const int direction, const size_t channel) const override {
    std::vector<std::string> antennas;
    antennas.push_back("RX");
    return antennas;
  }

  void setAntenna(const int direction, const size_t channel, const std::string &name) override {
    if (direction != SOAPY_SDR_RX) {
      throw std::runtime_error("setAntenna failed: Dummy Radio only supports RX");
    }
  }

  std::string getAntenna(const int direction, const size_t channel) const override { return "RX"; }

  /*******************************************************************
   * Frontend corrections API
   ******************************************************************/
  bool hasDCOffsetMode(const int direction, const size_t channel) const override { return false; }

  bool hasFrequencyCorrection(const int direction, const size_t channel) const override { return false; }

  void setFrequencyCorrection(const int direction, const size_t channel, const double value) override {}

  double getFrequencyCorrection(const int direction, const size_t channel) const override { return 0.; }

  /*******************************************************************
   * Gain API
   ******************************************************************/

  std::vector<std::string> listGains(const int direction, const size_t channel) const override {
    std::vector<std::string> results{};
    results.push_back("TUNER");
    return results;
  }

  bool hasGainMode(const int direction, const size_t channel) const override { return true; }

  void setGainMode(const int direction, const size_t channel, const bool automatic) override {
    gain_mode_ = automatic;
    SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting Dummy Radio gain mode: %s", automatic ? "Automatic" : "Manual");
  }

  bool getGainMode(const int direction, const size_t channel) const override { return gain_mode_; }

  void setGain(const int direction, const size_t channel, const double value) override {
    // TODO(james): Determine if this is helpful at all.
    SoapySDR::Device::setGain(direction, channel, value);
  }

  void setGain(const int direction, const size_t channel, const std::string &name, const double value) override {
    if (name == "TUNER") {
      tuner_gain_ = value;
      SoapySDR_logf(SOAPY_SDR_DEBUG, "Setting Dummy Radio Tuner Gain: %f", tuner_gain_);
    }
  }

  double getGain(const int direction, const size_t channel, const std::string &name) const override {
    if (name == "TUNER") {
      return tuner_gain_;
    }

    return 0.;
  }

  SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const override {
    return SoapySDR::Range(GAIN_MIN, GAIN_MAX);
  }

  /*******************************************************************
   * Frequency API
   ******************************************************************/

  void setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency,
                    const SoapySDR::Kwargs &args) override {
    if (name == "RF") {
      SoapySDR::logf(SOAPY_SDR_DEBUG, "Setting center freq: %d", (uint32_t)frequency);
      center_frequency_ = frequency;
    }
  }

  double getFrequency(const int direction, const size_t channel, const std::string &name) const override {
    if (name == "RF") {
      return center_frequency_;
    }

    return 0;
  }

  std::vector<std::string> listFrequencies(const int direction, const size_t channel) const override {
    std::vector<std::string> names;
    names.push_back("RF");
    return names;
  }

  SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel,
                                        const std::string &name) const override {
    SoapySDR::RangeList results;
    if (name == "RF") {
      results.push_back(SoapySDR::Range(20'000'000, 3'000'000'000));
    }
    return results;
  }

  SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const override {
    SoapySDR::ArgInfoList freqArgs{};
    return freqArgs;
  }

 private:
  double center_frequency_{};
  double tuner_gain_{GAIN_MIN};
  bool gain_mode_{false};
};

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
