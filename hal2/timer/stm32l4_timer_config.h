#pragma once

#include "hal2/device.h"
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

template <typename System, typename DeviceRegistry, size_t INDEX>
class Stm32L4LowPowerTimerAccessor;

template <typename System, typename DeviceRegistry>
class Stm32L4LowPowerTimerAccessor<System, DeviceRegistry, 0> {
 private:
  static constexpr DeviceId lsi_id{Facility::OSCILLATOR_FACILITY,
                                               LSI_OSCILLATOR_INDEX};

  using LsiOscillatorAccessor = config::accessor_type<System, DeviceRegistry, lsi_id>;

  Stm32l4xxLptim timer_{0, static_cast<LsiOscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] TimerDevice& get() noexcept { return timer_; }
  [[nodiscard]] TimerDevice& lp_timer() noexcept { return get(); }
  [[nodiscard]] TimerDevice& timer0() noexcept { return get(); }
  [[nodiscard]] TimerDevice& lp_timer0() noexcept { return get(); }
};

template <typename System, typename DeviceRegistry>
class Stm32L4LowPowerTimerAccessor<System, DeviceRegistry, 1> {
 private:
  static constexpr DeviceId lsi_id{Facility::OSCILLATOR_FACILITY,
                                               LSI_OSCILLATOR_INDEX};

  using LsiOscillatorAccessor = config::accessor_type<System, DeviceRegistry, lsi_id>;

  Stm32l4xxLptim timer_{1, static_cast<LsiOscillatorAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] TimerDevice& get() noexcept { return timer_; }
  [[nodiscard]] TimerDevice& timer1() noexcept { return get(); }
  [[nodiscard]] TimerDevice& lp_timer1() noexcept { return get(); }
};

}  // namespace tvsc::hal2::timer
