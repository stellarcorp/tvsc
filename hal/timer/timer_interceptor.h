#pragma once

#include "hal/interceptor.h"
#include "hal/timer/timer.h"

namespace tvsc::hal::timer {

class TimerInterceptor final : public Interceptor<TimerPeripheral> {
 public:
  TimerInterceptor(TimerPeripheral& timer) : Interceptor(timer) {}

  PeripheralId id() override;

  void start(uint32_t interval_us) override;
  void stop() override;

  bool is_running() override;

  // Turn on timer and clock to this peripheral.
  void enable() override;
  void disable() override;

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::timer
