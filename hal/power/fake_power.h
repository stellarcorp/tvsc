#pragma once

#include "hal/power/power.h"
#include "hal/simulation/reactor.h"

namespace tvsc::hal::power {

template <typename ClockType>
class FakePower final : public Power {
 private:
  simulation::Reactor<ClockType>* reactor_;

 public:
  FakePower(simulation::Reactor<ClockType>& reactor) : reactor_(&reactor) {}

  void enter_sleep_mode() override { reactor_->block_core_thread_until_irq(); }

  void enter_stop_mode() override {
    // TODO(james): Also turn off IRQ generations for facilities that do not run in stop mode.
    reactor_->block_core_thread_until_irq();
  }
};

}  // namespace tvsc::hal::power
