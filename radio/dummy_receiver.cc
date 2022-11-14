#include <chrono>
#include <limits>
#include <string>
#include <vector>

#include "SoapySDR/Constants.h"
#include "SoapySDR/Device.hpp"
#include "SoapySDR/Formats.hpp"
#include "SoapySDR/Logger.hpp"
#include "SoapySDR/Modules.hpp"
#include "SoapySDR/Registry.hpp"
#include "SoapySDR/Time.hpp"
#include "SoapySDR/Types.hpp"
#include "buffer/buffer.h"
#include "io/looping_file_reader.h"
#include "transform/pcm_to_iq.h"

// getModuleLoading() is actually a private/hidden function in SoapySDR, but we use it during registration to catch a
// common link error.
std::string &getModuleLoading(void);

namespace tvsc::radio {

constexpr double GAIN_MIN{0.};
constexpr double GAIN_MAX{1.};

constexpr int BUFFER_SIZE{1024};
constexpr int NUM_BUFFERS{32};

constexpr size_t MTU{4096};

// Filename of a file containing signed 16-bit little-endian PCM data to act as a mock signal being received.
constexpr char MOCK_RECEIVED_SIGNAL_FILENAME[]{"services/radio/server/modules/received_signal.pcm"};

//#define TLOG(...) SoapySDR::logf(SOAPY_SDR_TRACE __VA_OPT__(, ) __VA_ARGS__)
#define TLOG(...) fprintf(stderr __VA_OPT__(, ) __VA_ARGS__)

class DummyReceiverDevice final : public SoapySDR::Device {
 public:
  DummyReceiverDevice() {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    fprintf(stderr, "DummyReceiverDevice()\n");
  }

  std::string getDriverKey() const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return "dummy_receiver";
  }

  std::string getHardwareKey() const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return "TVSC Dummy Receiver";
  }

  SoapySDR::Kwargs getHardwareInfo() const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    SoapySDR::Kwargs args{};

    args["origin"] = "https://stellarcorp.tv/";
    args["index"] = "0";
    args["label"] = "TVSC Dummy Receiver";

    return args;
  }

  /*******************************************************************
   * Channels API
   ******************************************************************/
  size_t getNumChannels(int direction) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (direction == SOAPY_SDR_TX) {
      return 0;
    } else if (direction == SOAPY_SDR_RX) {
      return 1;
    } else {
      return -1;
    }
  }

  bool getFullDuplex(int direction, size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return false;
  }

  /*******************************************************************
   * Antenna API
   ******************************************************************/
  std::vector<std::string> listAntennas(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    std::vector<std::string> antennas;
    antennas.push_back("RX");
    return antennas;
  }

  void setAntenna(const int direction, const size_t channel, const std::string &name) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (direction != SOAPY_SDR_RX) {
      throw std::runtime_error("setAntenna failed: Dummy Receiver only supports RX");
    }
  }

  std::string getAntenna(const int direction, const size_t channel) const override { return "RX"; }

  /*******************************************************************
   * Frontend corrections API
   ******************************************************************/
  bool hasDCOffsetMode(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return false;
  }

  bool hasFrequencyCorrection(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return false;
  }

  void setFrequencyCorrection(const int direction, const size_t channel, const double value) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
  }

  double getFrequencyCorrection(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);

    return 0.;
  }

  /*******************************************************************
   * Gain API
   ******************************************************************/
  std::vector<std::string> listGains(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    std::vector<std::string> results{};
    results.push_back("TUNER");
    return results;
  }

  bool hasGainMode(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return true;
  }

  void setGainMode(const int direction, const size_t channel, const bool automatic) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    gain_mode_ = automatic;
    SoapySDR::logf(SOAPY_SDR_DEBUG, "Setting Dummy Receiver gain mode: %s", automatic ? "Automatic" : "Manual");
  }

  bool getGainMode(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return gain_mode_;
  }

  void setGain(const int direction, const size_t channel, const double value) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    // TODO(james): Determine if this is helpful at all.
    SoapySDR::Device::setGain(direction, channel, value);
  }

  void setGain(const int direction, const size_t channel, const std::string &name, const double value) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (name == "TUNER") {
      tuner_gain_ = value;
      SoapySDR::logf(SOAPY_SDR_DEBUG, "Setting Dummy Receiver Tuner Gain: %f", tuner_gain_);
    }
  }

  double getGain(const int direction, const size_t channel, const std::string &name) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (name == "TUNER") {
      return tuner_gain_;
    }

    return 0.;
  }

  SoapySDR::Range getGainRange(const int direction, const size_t channel, const std::string &name) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return SoapySDR::Range(GAIN_MIN, GAIN_MAX);
  }

  /*******************************************************************
   * Frequency API
   ******************************************************************/
  void setFrequency(const int direction, const size_t channel, const std::string &name, const double frequency,
                    const SoapySDR::Kwargs &args) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (name == "RF") {
      SoapySDR::logf(SOAPY_SDR_DEBUG, "Setting center freq: %d", (uint32_t)frequency);
      center_frequency_ = frequency;
    }
  }

  double getFrequency(const int direction, const size_t channel, const std::string &name) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (name == "RF") {
      return center_frequency_;
    }

    return 0;
  }

  std::vector<std::string> listFrequencies(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    std::vector<std::string> names;
    names.push_back("RF");
    return names;
  }

  SoapySDR::RangeList getFrequencyRange(const int direction, const size_t channel,
                                        const std::string &name) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    SoapySDR::RangeList results;
    if (name == "RF") {
      results.push_back(SoapySDR::Range(20'000'000, 3'000'000'000));
    }
    return results;
  }

  SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    SoapySDR::ArgInfoList freqArgs{};
    return freqArgs;
  }

  /*******************************************************************
   * Sample Rate API
   ******************************************************************/
  void setSampleRate(const int direction, const size_t channel, const double rate) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    sample_rate_ = rate;
  }

  double getSampleRate(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return sample_rate_;
  }

  std::vector<double> listSampleRates(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    std::vector<double> results;

    results.push_back(250000);
    results.push_back(1024000);
    results.push_back(1536000);
    results.push_back(1792000);
    results.push_back(1920000);
    results.push_back(2048000);
    results.push_back(2160000);
    results.push_back(2560000);
    results.push_back(2880000);
    results.push_back(3200000);

    return results;
  }

  SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    SoapySDR::RangeList results;

    results.push_back(SoapySDR::Range(225001, 300000));
    results.push_back(SoapySDR::Range(900001, 3200000));

    return results;
  }

  void setBandwidth(const int direction, const size_t channel, const double bw) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    bandwidth_ = bw;
  }

  double getBandwidth(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (bandwidth_ == 0) {
      // auto / full bandwidth
      return sample_rate_;
    }
    return bandwidth_;
  }

  std::vector<double> listBandwidths(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return std::vector<double>{};
  }

  SoapySDR::RangeList getBandwidthRange(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    SoapySDR::RangeList results{};

    // TODO(james): Determine real values here.
    results.push_back(SoapySDR::Range(0, 8'000'000));

    return results;
  }

  /*******************************************************************
   * Time API
   ******************************************************************/
  std::vector<std::string> listTimeSources(void) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    std::vector<std::string> results{};
    results.push_back("sw_ticks");
    return results;
  }

  std::string getTimeSource(void) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return "sw_ticks";
  }

  bool hasHardwareTime(const std::string &what) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return what == "" || what == "sw_ticks";
  }

  long long getHardwareTime(const std::string &what) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    return SoapySDR::ticksToTimeNs(ticks_, sample_rate_);
  }

  void setHardwareTime(const long long timeNs, const std::string &what) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    ticks_ = SoapySDR::timeNsToTicks(timeNs, sample_rate_);
  }

  /*******************************************************************
   * Settings API
   ******************************************************************/
  SoapySDR::ArgInfoList getSettingInfo(void) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    SoapySDR::ArgInfoList setArgs{};

    SoapySDR::ArgInfo testModeArg{};

    testModeArg.key = "testmode";
    testModeArg.value = "false";
    testModeArg.name = "Test Mode";
    testModeArg.description = "Test Mode";
    testModeArg.type = SoapySDR::ArgInfo::BOOL;

    setArgs.push_back(testModeArg);

    return setArgs;
  }

  void writeSetting(const std::string &key, const std::string &value) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (key == "testmode") {
      test_mode_ = (value == "true");
      SoapySDR::logf(SOAPY_SDR_DEBUG, "Dummy receiver test mode: %s", test_mode_ ? "true" : "false");
    }
  }

  std::string readSetting(const std::string &key) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (key == "testmode") {
      return test_mode_ ? "true" : "false";
    }

    SoapySDR::logf(SOAPY_SDR_WARNING, "Unknown setting '%s'", key.c_str());
    return "";
  }

  /*******************************************************************
   * Stream API
   ******************************************************************/
  std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    std::vector<std::string> formats{};

    formats.push_back(SOAPY_SDR_CF32);

    return formats;
  }

  std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (direction != SOAPY_SDR_RX) {
      throw std::runtime_error("Dummy Receiver is RX only");
    }

    fullScale = std::numeric_limits<short>::max();
    return SOAPY_SDR_CF32;
  }

  SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    if (direction != SOAPY_SDR_RX) {
      throw std::runtime_error("Dummy Receiver is RX only");
    }

    // TODO(james): Adjust to allow configuration of the stream: bps, format, etc.
    return SoapySDR::ArgInfoList{};
  }

  SoapySDR::Stream *setupStream(const int direction, const std::string &format, const std::vector<size_t> &channels,
                                const SoapySDR::Kwargs &args) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    // check the channel configuration
    if (channels.size() != 1 or channels.at(0) != 0) {
      throw std::runtime_error("setupStream invalid channel selection");
    }

    // check the format
    if (format == SOAPY_SDR_S16) {
      TLOG("Using format S16.\n");
    } else if (format == SOAPY_SDR_CF32) {
      TLOG("Using format CF32.\n");
    } else {
      TLOG("Invalid format %s requested\n", format.c_str());
      throw std::runtime_error("setupStream invalid format '" + format +
                               "' -- Only S16 is supported by the Dummy Receiver module.");
    }

    stream_.reset(new tvsc::io::LoopingFileReader<short>{MOCK_RECEIVED_SIGNAL_FILENAME});
    return reinterpret_cast<SoapySDR::Stream *>(stream_.get());
  }

  void closeStream(SoapySDR::Stream *stream) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    stream_.reset();
  }

  size_t getStreamMTU(SoapySDR::Stream *stream) const override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    // Typical block size for a disk-based file. The exact number here is not too important.
    return MTU;
  }

  int activateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs,
                     const size_t numElems) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    // Nothing to do. Our ring buffer fills itself when it needs data. So, no extraneous work happens when we are not
    // reading off the data.
    return 0;
  }

  int deactivateStream(SoapySDR::Stream *stream, const int flags, const long long timeNs) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    // Nothing to do. Our ring buffer fills itself when it needs data. So, no extraneous work happens when we are not
    // reading off the data.
    return 0;
  }

  int readStream(SoapySDR::Stream *stream, void *const *buffs, const size_t numElems, int &flags, long long &timeNs,
                 const long timeoutUs) override {
    TLOG("%s:%d DummyReceiverDevice::%s\n", __FILE__, __LINE__, __func__);
    size_t elements_to_read = std::min(numElems, MTU);
    size_t elements_read = stream_->read(elements_to_read, buffer_.data());
    tvsc::transform::psm_s16le_to_iq(buffer_, elements_read, static_cast<std::complex<float> *>(buffs[0]));
    return elements_read;
  }

 private:
  double center_frequency_{};
  double tuner_gain_{GAIN_MIN};
  double sample_rate_{0.};
  double bandwidth_{0.};
  long ticks_{0};
  bool gain_mode_{false};
  bool test_mode_{false};

  tvsc::buffer::Buffer<short, MTU> buffer_{};
  std::unique_ptr<tvsc::io::LoopingFileReader<short>> stream_{};
};

SoapySDR::KwargsList find_dummy_receiver(const SoapySDR::Kwargs &args) {
  SoapySDR::log(SOAPY_SDR_DEBUG, "find_dummy_receiver() -- args:");
  for (const auto &arg : args) {
    SoapySDR::logf(SOAPY_SDR_DEBUG, "\targ.first: %s, arg.second: %s", arg.first, arg.second);
  }
  SoapySDR::Kwargs device_info;
  device_info["label"] = "TVSC Dummy Receiver";
  device_info["product"] = "";
  device_info["serial"] = "";
  device_info["manufacturer"] = "TVSC";
  device_info["tuner"] = "RF";
  SoapySDR::KwargsList results{};
  results.push_back(device_info);
  return results;
}

SoapySDR::Device *make_dummy_receiver(const SoapySDR::Kwargs &args) {
  SoapySDR::log(SOAPY_SDR_DEBUG, "make_dummy_receiver() -- args:");
  for (const auto &arg : args) {
    SoapySDR::logf(SOAPY_SDR_DEBUG, "\targ.first: %s, arg.second: %s", arg.first, arg.second);
  }
  return new DummyReceiverDevice{};
}

void register_dummy_receiver() {
  static SoapySDR::Registry module_registration{"dummy_receiver", &find_dummy_receiver, &make_dummy_receiver,
                                                SOAPY_SDR_ABI_VERSION};
  static SoapySDR::ModuleVersion module_version{SOAPY_SDR_ABI_VERSION};

  // If the SoapySDR library does not have the correct linkage, we get a situation where the registration code has
  // multiple copies of the static variables, such as the current module being loaded. This block of code identifies
  // that scenario and forces an immediate failure with a log message so that the build issue can be more easily
  // identified.
  const std::string &current_module{getModuleLoading()};
  if (current_module.empty()) {
    SoapySDR::log(SOAPY_SDR_FATAL, "register_dummy_receiver() -- current_module is empty during module registration");
    abort();
  } else {
    TLOG("register_dummy_receiver() -- current_module: %s", current_module.c_str());
  }
}

}  // namespace tvsc::radio
