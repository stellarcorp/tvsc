#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>

#include "hal/imu/imu.h"
#include "scheduler/task.h"

namespace tvsc::bringup {

using namespace std::chrono_literals;

struct alignas(16) ImuReading final {
  alignas(4) uint16_t device_id{0xffff};
  bool last_read_result{false};
  alignas(16) std::array<int16_t, 3> accel_raw{
      static_cast<int16_t>(0xffff), static_cast<int16_t>(0xffff), static_cast<int16_t>(0xffff)};
  alignas(16) std::array<float, 3> accel{};
  alignas(16) std::array<int16_t, 3> gyro_raw{
      static_cast<int16_t>(0xffff), static_cast<int16_t>(0xffff), static_cast<int16_t>(0xffff)};
  alignas(16) std::array<float, 3> gyro{};

  void reset() { *this = ImuReading{}; }
};

template <typename ClockType>
tvsc::scheduler::Task<ClockType> monitor_imu(tvsc::hal::imu::ImuPeripheral& imu_peripheral,
                                             ImuReading& output,
                                             std::chrono::microseconds interval = 0s) {
  using namespace std::chrono_literals;

  const std::chrono::microseconds startup_delay{5ms};

  if (interval > startup_delay) {
    while (true) {
      {
        // Create the functional in its own block so that the monitor's resources can be shutdown
        // when we aren't actively reading measurements.
        auto imu{imu_peripheral.access()};
        co_yield startup_delay;
        output.reset();
        bool last_read_result{true};
        last_read_result &= imu.read_id(output.device_id);
        last_read_result &= imu.read_accelerometer(output.accel, &output.accel_raw);
        last_read_result &= imu.read_gyroscope(output.gyro, &output.gyro_raw);
        output.last_read_result = last_read_result;
        imu.put_in_standby_mode();
      }

      co_yield interval - startup_delay;
    }
  } else {
    // Just leave the functional on all the time, as this function is essentially being asked to
    // take measurements constantly.

    const std::chrono::microseconds measurement_time{5ms};

    auto imu{imu_peripheral.access()};
    co_yield startup_delay;
    while (true) {
      output.reset();
      bool last_read_result{true};
      last_read_result &= imu.read_id(output.device_id);
      last_read_result &= imu.read_accelerometer(output.accel, &output.accel_raw);
      last_read_result &= imu.read_gyroscope(output.gyro, &output.gyro_raw);
      output.last_read_result = last_read_result;
      co_yield std::max(measurement_time, interval);
    }
  }
}

}  // namespace tvsc::bringup
