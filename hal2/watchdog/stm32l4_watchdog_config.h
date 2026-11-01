#pragma once

#include "hal2/peripheral.h"
#include "hal2/rcc/oscillator.h"
#include "hal2/watchdog/stm32l4_watchdog.h"
#include "hal2/watchdog/watchdog.h"

namespace tvsc::hal2::watchdog {

// The index part of the peripheral id for the LSI oscillator.
// TODO(james): Figure out a better pattern for dependencies. It's nice that we know how to pick up
// an oscillator by its Facility value, but we need a better way to designate specific indices of a
// Facility for specific tasks.
// TODO(james): Determine what value this should have if both LP watchdogs are in use.

template <typename System, typename PeripheralRegistry, auto configuration>
class Stm32L4WatchdogAccessor {
 private:
  static constexpr size_t LSI_OSCILLATOR_INDEX{1};
  static constexpr PeripheralId LSI_OSCILLATOR_ID{Facility::OSCILLATOR_FACILITY,
                                                          LSI_OSCILLATOR_INDEX};
  using LsiOscillatorAccessor =
      config::accessor_type<System, PeripheralRegistry, LSI_OSCILLATOR_ID>;

  Stm32L4Watchdog watchdog_{static_cast<LsiOscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] WatchdogPeripheral& get() noexcept { return watchdog_; }
  [[nodiscard]] auto& watchdog0() noexcept { return get(); }
};

}  // namespace tvsc::hal2::watchdog
