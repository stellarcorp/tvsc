#pragma once

#include "hal/simulation/interceptor.h"
#include "hal/timer/timer.h"

namespace tvsc::hal::timer {

template <typename ClockType>
class TimerInterceptor final : public simulation::Interceptor<TimerPeripheral, ClockType> {
 public:
  TimerInterceptor(TimerPeripheral& timer) : simulation::Interceptor<TimerPeripheral, ClockType>(timer) {}

  PeripheralId id() override {
    LOG_FN();
    return this->call(&TimerPeripheral::id);
  }

  void start(uint32_t interval_us) override {
    LOG_FN();
    return this->call(&TimerPeripheral::start, interval_us);
  }

  void stop() override {
    LOG_FN();
    return this->call(&TimerPeripheral::stop);
  }

  bool is_running() override {
    LOG_FN();
    return this->call(&TimerPeripheral::is_running);
  }

  void enable() override {
    LOG_FN();
    return this->call(&TimerPeripheral::enable);
  }

  void disable() override {
    LOG_FN();
    return this->call(&TimerPeripheral::disable);
  }

  void handle_interrupt() override {
    LOG_FN();
    return this->call(&TimerPeripheral::handle_interrupt);
  }
};

}  // namespace tvsc::hal::timer
