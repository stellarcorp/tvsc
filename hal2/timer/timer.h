#pragma once

#include <cstddef>

#include "hal2/device.h"

namespace tvsc::hal2::timer {

class Timer;

class TimerDevice : public Device<TimerDevice, Timer> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void configure_single_pulse(uint32_t interval_us) noexcept = 0;
  virtual void start() noexcept = 0;
  virtual void stop() noexcept = 0;

  [[nodiscard]] virtual bool is_running() noexcept = 0;

  friend class Timer;

  template <typename ClockType>
  friend class TimerInterceptor;

 public:
  virtual ~TimerDevice() = default;
};

class Timer final : public Driver<TimerDevice, Timer> {
 protected:
  explicit Timer(TimerDevice& peripheral) noexcept
      : Driver<TimerDevice, Timer>(peripheral) {}

  friend class Device<TimerDevice, Timer>;

 public:
  Timer() noexcept = default;

  void configure_single_pulse(uint32_t interval_us) noexcept {
    device_->configure_single_pulse(interval_us);
  }
  void start() noexcept { device_->start(); }
  void stop() noexcept { device_->stop(); }

  [[nodiscard]] bool is_running() noexcept { return device_->is_running(); }
};

}  // namespace tvsc::hal2::timer
