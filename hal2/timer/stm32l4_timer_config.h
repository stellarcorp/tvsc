#pragma once

#include "hal2/rcc/oscillator.h"
#include "hal2/timer/stm32l4_timer.h"
#include "hal2/timer/timer.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::timer {

// The index part of the peripheral id for the LSI oscillator.
// TODO(james): Figure out a better pattern for dependencies. It's nice that we know how to pick up
// an oscillator by its Facility value, but we need a better way to designate specific indices of a
// Facility for specific tasks.
// TODO(james): Determine what value this should have if both LP timers are in use.
static constexpr size_t LSI_OSCILLATOR_INDEX{1};

template <typename System, typename PeripheralRegistry, size_t INDEX>
class Stm32L4LowPowerTimerAccessor;

template <typename System, typename PeripheralRegistry>
class Stm32L4LowPowerTimerAccessor<System, PeripheralRegistry, 0> {
 private:
  using LsiOscillatorAccessor =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::OSCILLATOR_FACILITY,
          LSI_OSCILLATOR_INDEX>::template accessor<System, PeripheralRegistry>;

  Stm32l4xxLptim timer_{0, static_cast<LsiOscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] TimerPeripheral& get() noexcept { return timer_; }
  [[nodiscard]] TimerPeripheral& lp_timer() noexcept { return get(); }
  [[nodiscard]] TimerPeripheral& timer0() noexcept { return get(); }
  [[nodiscard]] TimerPeripheral& lp_timer0() noexcept { return get(); }
};

template <typename System, typename PeripheralRegistry>
class Stm32L4LowPowerTimerAccessor<System, PeripheralRegistry, 1> {
 private:
  using LsiOscillatorAccessor =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::OSCILLATOR_FACILITY,
          LSI_OSCILLATOR_INDEX>::template accessor<System, PeripheralRegistry>;

  Stm32l4xxLptim timer_{1, static_cast<LsiOscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] TimerPeripheral& get() noexcept { return timer_; }
  [[nodiscard]] TimerPeripheral& timer1() noexcept { return get(); }
  [[nodiscard]] TimerPeripheral& lp_timer1() noexcept { return get(); }
};

}  // namespace tvsc::hal2::timer
