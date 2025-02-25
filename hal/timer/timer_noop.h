#pragma once

#include "hal/timer/timer.h"

namespace tvsc::hal::timer {

class TimerNoop final : public TimerPeripheral {
 public:
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
