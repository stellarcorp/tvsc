#pragma once

#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

template <typename ClockT>
class FakeSysTick final : public SysTickType {
 public:
  using ClockType = ClockT;

 private:
  TimeType uwTick_{};

 public:
  TimeType current_time_micros() override { return uwTick_; }
  void increment_micros(TimeType us) override { uwTick_ += us; }
  void handle_interrupt() override { uwTick_ += 1000; }
};

}  // namespace tvsc::hal::systick
