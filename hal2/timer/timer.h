#pragma once

#include <cstddef>

#include "hal2/peripheral.h"

namespace tvsc::hal2::timer {

class Timer;

class TimerPeripheral : public Peripheral<TimerPeripheral, Timer> {
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
  virtual ~TimerPeripheral() = default;
};

class Timer final : public Driver<TimerPeripheral, Timer> {
 protected:
  explicit Timer(TimerPeripheral& peripheral) noexcept
      : Driver<TimerPeripheral, Timer>(peripheral) {}

  friend class Peripheral<TimerPeripheral, Timer>;

 public:
  Timer() noexcept = default;

  void configure_single_pulse(uint32_t interval_us) noexcept {
    peripheral_->configure_single_pulse(interval_us);
  }
  void start() noexcept { peripheral_->start(); }
  void stop() noexcept { peripheral_->stop(); }

  [[nodiscard]] bool is_running() noexcept { return peripheral_->is_running(); }
};

}  // namespace tvsc::hal2::timer
