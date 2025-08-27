#pragma once

#include <cstdint>

#include "hal/gpio/gpio.h"
#include "hal/peripheral.h"
#include "hal/peripheral_id.h"

namespace tvsc::hal::timer {

class Timer;

class TimerPeripheral : public Peripheral<TimerPeripheral, Timer> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  // Peripheral id for linking peripherals together in a platform-dependent manner.
  virtual PeripheralId id() = 0;

  virtual void start(uint32_t interval_us) = 0;
  virtual void stop() = 0;

  virtual bool is_running() = 0;

  friend class Timer;

  template <typename ClockType>
  friend class TimerInterceptor;

 public:
  virtual ~TimerPeripheral() = default;

  virtual void handle_interrupt() = 0;
};

class Timer final : public Functional<TimerPeripheral, Timer> {
 protected:
  explicit Timer(TimerPeripheral& peripheral) : Functional<TimerPeripheral, Timer>(peripheral) {}

  friend class Peripheral<TimerPeripheral, Timer>;

 public:
  Timer() = default;

  PeripheralId id() { return peripheral_->id(); }

  void start(uint32_t interval_us) { peripheral_->start(interval_us); }
  void stop() { peripheral_->stop(); }

  bool is_running() { return peripheral_->is_running(); }

  void handle_interrupt() { peripheral_->handle_interrupt(); }
};

}  // namespace tvsc::hal::timer
