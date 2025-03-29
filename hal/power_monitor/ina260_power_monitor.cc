#include "hal/power_monitor/ina260_power_monitor.h"

#include "hal/error.h"

namespace tvsc::hal::power_monitor {

void Ina260PowerMonitor::enable() {
  i2c_ = i2c_peripheral_->access();
  require(i2c_.is_device_ready(addr_));

  require(configure());
}

void Ina260PowerMonitor::disable() {
  // shutdown();
  i2c_.invalidate();
}

bool Ina260PowerMonitor::configure() {
  // See https://www.ti.com/lit/ds/symlink/ina260.pdf, page 22, "Configuration Register", for more
  // details on this configuration value. Note that the atypical separator structure mirrors the
  // bitfields of the register.
  static constexpr uint16_t configuration_value{0b0'110'000'100'100'111};
  static constexpr uint8_t configuration_bytes[2]{static_cast<uint8_t>(configuration_value >> 8),
                                                  static_cast<uint8_t>(configuration_value & 0xff)};

  bool success{i2c_.write(addr_, CONFIGURATION_REGISTER, const_cast<uint8_t*>(configuration_bytes),
                          sizeof(configuration_bytes))};

  if (success) {
    uint8_t bytes[2]{};
    success =
        i2c_.read(addr_, CONFIGURATION_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes));
    if (success) {
      uint16_t result = (static_cast<uint16_t>(bytes[0]) << 8) + bytes[1];
      if (result != configuration_value) {
        success = false;
      }
    }
  }

  return success;
}

bool Ina260PowerMonitor::shutdown() {
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

bool Ina260PowerMonitor::read_current(float* result_amps) {
  uint8_t bytes[2]{};
  bool success{
      i2c_.read(addr_, CURRENT_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result_amps = 0.00125f * ((static_cast<uint16_t>(bytes[0]) << 8) + bytes[1]);
  }
  return success;
}

bool Ina260PowerMonitor::read_voltage(float* result_volts) {
  uint8_t bytes[2]{};
  bool success{
      i2c_.read(addr_, BUS_VOLTAGE_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result_volts = 0.00125f * (static_cast<int16_t>(bytes[0] << 8) + bytes[1]);
  }
  return success;
}

bool Ina260PowerMonitor::read_power(float* result_watts) {
  uint8_t bytes[2]{};
  bool success{i2c_.read(addr_, POWER_REGISTER, reinterpret_cast<uint8_t*>(bytes), sizeof(bytes))};
  if (success) {
    *result_watts = 0.010f * ((static_cast<uint16_t>(bytes[0]) << 8) + bytes[1]);
  }
  return success;
}

}  // namespace tvsc::hal::power_monitor
