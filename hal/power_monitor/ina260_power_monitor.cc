#include "hal/power_monitor/ina260_power_monitor.h"

#include "bits/bits.h"
#include "hal/error.h"

namespace tvsc::hal::power_monitor {

void Ina260PowerMonitor::enable() {
  i2c_ = i2c_peripheral_->access();
  require(i2c_.is_device_ready(addr_));

  require(configure());
}

void Ina260PowerMonitor::disable() { i2c_.invalidate(); }

bool Ina260PowerMonitor::configure() {
  using namespace tvsc::bits;

  // See https://www.ti.com/lit/ds/symlink/ina260.pdf, page 22, "Configuration Register", for more
  // details on this configuration value. Note that the atypical separator structure mirrors the
  // bit fields of the register and makes it easier to read the values of the bit fields.
  uint16_t configuration_value{0b0'110'000'000'000'111};

  // The measurement time is used to configure the conversion time of both the voltage measurement
  // and the current measurement. We could track two separate times, but that hasn't been required
  // so far.
  modify_bit_field<3, 3>(configuration_value,
                         static_cast<uint16_t>(current_measurement_time_configuration_));

  modify_bit_field<3, 6>(configuration_value,
                         static_cast<uint16_t>(voltage_measurement_time_configuration_));

  modify_bit_field<3, 9>(configuration_value,
                         static_cast<uint16_t>(sample_averaging_configuration_));

  uint8_t configuration_bytes[2]{static_cast<uint8_t>(configuration_value >> 8),
                                 static_cast<uint8_t>(configuration_value & 0xff)};

  bool success{
      i2c_.write(addr_, CONFIGURATION_REGISTER, configuration_bytes, sizeof(configuration_bytes))};

  return success;
}

bool Ina260PowerMonitor::read_configuration(uint16_t* configuration) {
  uint8_t bytes[2]{};
  bool success{
      i2c_.read(addr_, CONFIGURATION_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *configuration = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
  }
  return success;
}

bool Ina260PowerMonitor::put_in_standby_mode() {
  // See https://www.ti.com/lit/ds/symlink/ina260.pdf, page 22, "Configuration Register", for more
  // details on this configuration value. Note that the atypical separator structure mirrors the
  // bitfields of the register.
  static constexpr uint16_t configuration_value{0b0'110'000'000'000'000};
  static constexpr uint8_t configuration_bytes[2]{static_cast<uint8_t>(configuration_value >> 8),
                                                  static_cast<uint8_t>(configuration_value & 0xff)};

  bool success{i2c_.write(addr_, CONFIGURATION_REGISTER, const_cast<uint8_t*>(configuration_bytes),
                          sizeof(configuration_bytes))};
  return success;
}

bool Ina260PowerMonitor::read_id(uint16_t* result) {
  uint8_t bytes[2]{};
  bool success{i2c_.read(addr_, ID_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
  }
  return success;
}

bool Ina260PowerMonitor::read_current(float* result_amps, uint16_t* raw_result) {
  uint8_t bytes[2]{};
  bool success{
      i2c_.read(addr_, CURRENT_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result_amps = 0.00125f * ((static_cast<uint16_t>(bytes[0]) << 8) + bytes[1]);
    if (raw_result != nullptr) {
      *raw_result = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
    }
  }
  return success;
}

bool Ina260PowerMonitor::read_voltage(float* result_volts, uint16_t* raw_result) {
  uint8_t bytes[2]{};
  bool success{
      i2c_.read(addr_, BUS_VOLTAGE_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result_volts = 0.00125f * (static_cast<int16_t>(bytes[0] << 8) + bytes[1]);
    if (raw_result != nullptr) {
      *raw_result = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
    }
  }
  return success;
}

bool Ina260PowerMonitor::read_power(float* result_watts, uint16_t* raw_result) {
  uint8_t bytes[2]{};
  bool success{i2c_.read(addr_, POWER_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result_watts = 0.010f * ((static_cast<uint16_t>(bytes[0]) << 8) + bytes[1]);
    if (raw_result != nullptr) {
      *raw_result = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
    }
  }
  return success;
}

std::chrono::microseconds Ina260PowerMonitor::current_measurement_time() {
  return MEASUREMENT_TIMES[current_measurement_time_configuration_];
}

void Ina260PowerMonitor::set_current_measurement_time_approximate(
    std::chrono::microseconds duration) {
  auto best_delta{std::chrono::microseconds::max()};
  uint8_t best_match_index{};
  uint8_t index{};
  for (auto allowed_duration : MEASUREMENT_TIMES) {
    const auto delta{std::chrono::abs(allowed_duration - duration)};
    if (delta < best_delta) {
      best_delta = delta;
      best_match_index = index;
    }
    ++index;
  }
  current_measurement_time_configuration_ = best_match_index;
}

std::chrono::microseconds Ina260PowerMonitor::voltage_measurement_time() {
  return MEASUREMENT_TIMES[voltage_measurement_time_configuration_];
}

void Ina260PowerMonitor::set_voltage_measurement_time_approximate(
    std::chrono::microseconds duration) {
  auto best_delta{std::chrono::microseconds::max()};
  uint8_t best_match_index{};
  uint8_t index{};
  for (auto allowed_duration : MEASUREMENT_TIMES) {
    const auto delta{std::chrono::abs(allowed_duration - duration)};
    if (delta < best_delta) {
      best_delta = delta;
      best_match_index = index;
    }
    ++index;
  }
  voltage_measurement_time_configuration_ = best_match_index;
}

uint16_t Ina260PowerMonitor::sample_averaging() {
  return SAMPLE_AVERAGING_VALUES[sample_averaging_configuration_];
}

void Ina260PowerMonitor::set_sample_averaging_approximate(uint16_t num_samples) {
  auto best_delta{std::numeric_limits<uint16_t>::max()};
  uint8_t best_match_index{};
  uint8_t index{};
  for (auto allowed_samples : SAMPLE_AVERAGING_VALUES) {
    const auto delta{std::abs(allowed_samples - num_samples)};
    if (delta < best_delta) {
      best_delta = delta;
      best_match_index = index;
    }
    ++index;
  }
  sample_averaging_configuration_ = best_match_index;
}

}  // namespace tvsc::hal::power_monitor
