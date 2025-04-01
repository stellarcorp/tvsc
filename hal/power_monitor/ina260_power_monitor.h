#pragma once

#include <array>
#include <chrono>
#include <cstdint>

#include "hal/i2c/i2c.h"
#include "hal/power_monitor/power_monitor.h"

namespace tvsc::hal::power_monitor {

using namespace std::chrono_literals;

class Ina260PowerMonitor final : public PowerMonitorPeripheral {
 public:
  // Lookup array of supported measurement times. This is the maximum time the power monitor takes
  // to get a reading. Longer times tend to yield more accurate readings, but they take longer and
  // might lose the ability to detect transients. This value is referred to as "conversion time" in
  // the INA260 datasheet. The values in this array correspond to the maximum ADC conversion times
  // as listed in table 7.5 Electrical Characteristics on page 6 of the datasheet
  // (https://www.ti.com/lit/ds/symlink/ina260.pdf). Also, see the details on the Configuration
  // Register on page 22. Note that we explicitly chose the maximum times to so that we can initiate
  // a conversion, sleep or co_yield for the time in this array (possibly plus some factor for clock
  // skew, etc.), and then read a valid value, all without the need for configuring an interrupt. As
  // we currently do not plan to use these power monitors on our satellites, configuring and
  // managing an interrupt line for this part is likely a waste of our time.
  static constexpr std::array<std::chrono::microseconds, 8> MEASUREMENT_TIMES{
      154us, 224us, 365us, 646us, 1210us, 2328us, 4572us, 9068us};

  // Lookup array of supported sample averaging values. These are the number of samples that are
  // collected and then averaged together to produce a measurement. This value is referred to as
  // "averaging mode" in the INA260 datasheet. The values in this array correspond to the allowed
  // values used in the configuration register. See the details on the Configuration Register on
  // page 22 of https://www.ti.com/lit/ds/symlink/ina260.pdf.
  static constexpr std::array<uint16_t, 8> SAMPLE_AVERAGING_VALUES{1,   4,   16,  64,
                                                                   128, 256, 512, 1024};

 private:
  static constexpr uint8_t ID_REGISTER{0xff};

  static constexpr uint8_t CONFIGURATION_REGISTER{0x00};
  static constexpr uint8_t CURRENT_REGISTER{0x01};
  static constexpr uint8_t BUS_VOLTAGE_REGISTER{0x02};
  static constexpr uint8_t POWER_REGISTER{0x03};

  uint8_t addr_;
  i2c::I2cPeripheral* i2c_peripheral_;
  i2c::I2c i2c_{};
  // Index into the MEASUREMENT_TIMES array indicating the time for current measurements. Also, the
  // value that will be directly set in the bit field of the configuration register. The default
  // value is the default value of the bit field according to the datasheet.
  uint8_t current_measurement_time_configuration_{4};

  // Index into the MEASUREMENT_TIMES array indicating the time for voltage measurements. Also, the
  // value that will be directly set in the bit field of the configuration register. The default
  // value is the default value of the bit field according to the datasheet.
  uint8_t voltage_measurement_time_configuration_{4};

  // Index into the SAMPLE_AVERAGING_VALUES array. Also, the value that will be directly set in the
  // bit field of the configuration register.  The default value is the default value of the bit
  // field according to the datasheet.
  uint8_t sample_averaging_configuration_{0};

  void enable() override;
  void disable() override;

  // Configure the device to start (or resume) making measurements. This puts the device in
  // "Shunt Current and Bus Voltage, Continuous" mode as well as configuring the measurement time
  // (called "Conversion Time" in the datasheet) and sample averaging. See the details on the
  // Configuration Register on page 22 of https://www.ti.com/lit/ds/symlink/ina260.pdf.
  bool configure();

  bool read_configuration(uint16_t* result);

  bool read_id(uint16_t* result) override;
  bool read_current(float* result_amps, uint16_t* raw_result) override;
  bool read_voltage(float* result_volts, uint16_t* raw_result) override;
  bool read_power(float* result_watts, uint16_t* raw_result) override;

  std::chrono::microseconds current_measurement_time() override;
  void set_current_measurement_time_approximate(std::chrono::microseconds duration) override;

  std::chrono::microseconds voltage_measurement_time() override;
  void set_voltage_measurement_time_approximate(std::chrono::microseconds duration) override;

  uint16_t sample_averaging() override;
  void set_sample_averaging_approximate(uint16_t num_samples) override;

  // Put the power monitor in "Power-Down (or Shutdown)" mode. In this mode, the device stops making
  // measurements but still responds to bus messages. See the details on the Configuration Register
  // on page 22 of https://www.ti.com/lit/ds/symlink/ina260.pdf.
  bool put_in_standby_mode() override;

 public:
  Ina260PowerMonitor(uint8_t i2c_addr, i2c::I2cPeripheral& i2c_peripheral)
      : addr_(i2c_addr), i2c_peripheral_(&i2c_peripheral) {}
};

}  // namespace tvsc::hal::power_monitor
