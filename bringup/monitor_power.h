#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>

#include "hal/power_monitor/power_monitor.h"
#include "system/system.h"

namespace tvsc::bringup {

using namespace std::chrono_literals;

struct alignas(16) PowerUsage final {
  alignas(4) uint16_t device_id{0xffff};
  alignas(4) uint16_t current_raw{0xffff};
  alignas(4) uint16_t voltage_raw{0xffff};
  alignas(4) uint16_t power_raw{0xffff};
  float current_amps{};
  float voltage_volts{};
  float power_watts{};
  bool last_read_result{};

  void reset() { *this = PowerUsage{}; }
};

tvsc::system::System::Task monitor_power(
    tvsc::hal::power_monitor::PowerMonitorPeripheral& power_monitor_peripheral, PowerUsage& output,
    std::chrono::microseconds interval = 0s) {
  using namespace std::chrono_literals;

  const auto startup_delay{power_monitor_peripheral.sample_averaging() *
                               (power_monitor_peripheral.current_measurement_time() +
                                power_monitor_peripheral.voltage_measurement_time()) +
                           5ms};

  if (interval > startup_delay) {
    while (true) {
      {
        // Create the functional in its own block so that the monitor's resources can be shutdown
        // when we aren't actively reading measurements.
        auto power_monitor{power_monitor_peripheral.access()};
        co_yield startup_delay;
        output.reset();
        bool last_read_result{true};
        last_read_result &= power_monitor.read_id(&output.device_id);
        last_read_result &= power_monitor.read_current(&output.current_amps, &output.current_raw);
        last_read_result &= power_monitor.read_voltage(&output.voltage_volts, &output.voltage_raw);
        last_read_result &= power_monitor.read_power(&output.power_watts, &output.power_raw);
        output.last_read_result = last_read_result;
        power_monitor.put_in_standby_mode();
      }

      co_yield interval - startup_delay;
    }
  } else {
    // Just leave the functional on all the time, as this function is essentially being asked to
    // take measurements constantly.

    const auto measurement_time{power_monitor_peripheral.current_measurement_time() +
                                power_monitor_peripheral.voltage_measurement_time()};

    auto power_monitor{power_monitor_peripheral.access()};
    co_yield startup_delay;
    while (true) {
      bool last_read_result{true};
      output.reset();
      last_read_result &= power_monitor.read_id(&output.device_id);
      last_read_result &= power_monitor.read_current(&output.current_amps, &output.current_raw);
      last_read_result &= power_monitor.read_voltage(&output.voltage_volts, &output.voltage_raw);
      last_read_result &= power_monitor.read_power(&output.power_watts, &output.power_raw);
      output.last_read_result = last_read_result;
      co_yield std::max(measurement_time, interval);
    }
  }
}

}  // namespace tvsc::bringup
