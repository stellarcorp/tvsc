#pragma once

#include "hal/simulation/event_generator.h"
#include "hal/simulation/reactor.h"
#include "hal/systick/systick.h"
#include "hal/time_type.h"

namespace tvsc::hal::systick {

template <typename ClockT>
class FakeSysTick final : public SysTickType, public simulation::EventGenerator<ClockT> {
 public:
  using ClockType = ClockT;
  using ReactorType = simulation::Reactor<ClockType>;

 private:
  TimeType uwTick_{};

 public:
  FakeSysTick(ReactorType& reactor) { reactor.add_generator(*this); }

  TimeType current_time_micros() override { return uwTick_; }
  void increment_micros(TimeType us) override { uwTick_ += us; }
  void handle_interrupt() override { uwTick_ += 1000; }

  ClockType::duration next_event_in(ClockType::time_point /*now*/) const noexcept override {
    using namespace std::chrono_literals;
    return 1000us;
  }

  void generate(ClockType::time_point /*now*/) noexcept override { handle_interrupt(); }
};

}  // namespace tvsc::hal::systick
