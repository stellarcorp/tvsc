#pragma once

#include <chrono>
#include <cstdint>

#include "hal/board/board.h"
#include "hal/power_monitor/power_monitor.h"
#include "scheduler/task.h"

namespace tvsc::bringup {

using namespace std::chrono_literals;

struct alignas(16) PowerUsage final {
  uint16_t device_id{};
  float current_amps{};
  float voltage_volts{};
  float power_watts{};
};

template <typename ClockType>
tvsc::scheduler::Task<ClockType> monitor_power(
    tvsc::hal::power_monitor::PowerMonitorPeripheral& power_monitor_peripheral, PowerUsage& output,
    typename ClockType::duration interval = 1s) {
  while (true) {
    {
      // Create the functional in its own block so that the monitor's resources can be shutdown when
      // we aren't actively reading measurements.
      auto power_monitor{power_monitor_peripheral.access()};
      co_yield 5ms;
      power_monitor.read_id(&output.device_id);
      power_monitor.read_current(&output.current_amps);
      power_monitor.read_voltage(&output.voltage_volts);
      power_monitor.read_power(&output.power_watts);
    }

    co_yield interval - 5ms;
  }
}

}  // namespace tvsc::bringup
