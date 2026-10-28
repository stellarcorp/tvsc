#pragma once

#include "hal2/config/facility.h"
#include "hal2/rcc/oscillator.h"
#include "hal2/watchdog/stm32l4_watchdog.h"
#include "hal2/watchdog/watchdog.h"

namespace tvsc::hal2::watchdog {

// The index part of the peripheral id for the LSI oscillator.
// TODO(james): Figure out a better pattern for dependencies. It's nice that we know how to pick up
// an oscillator by its Facility value, but we need a better way to designate specific indices of a
// Facility for specific tasks.
// TODO(james): Determine what value this should have if both LP watchdogs are in use.
static constexpr size_t LSI_OSCILLATOR_INDEX{1};

template <typename System, typename PeripheralRegistry>
class Stm32L4WatchdogAccessor {
 private:
  using LsiOscillatorAccessor =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::OSCILLATOR_FACILITY,
          LSI_OSCILLATOR_INDEX>::template accessor<System, PeripheralRegistry>;

  Stm32L4Watchdog watchdog_{static_cast<LsiOscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] WatchdogPeripheral& get() noexcept { return watchdog_; }
  [[nodiscard]] auto& watchdog0() noexcept { return get(); }
};

}  // namespace tvsc::hal2::watchdog
