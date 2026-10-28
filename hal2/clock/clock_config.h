#pragma once

#include "hal2/clock/clock.h"
#include "hal2/config/facility.h"

namespace tvsc::hal2::clock {

template <typename System, typename PeripheralRegistry, size_t INDEX,
          SleepStrategy SLEEP_STRATEGY = SleepStrategy::SLEEP>
  requires(INDEX == 0)
class ClockAccessor {
 private:
  static constexpr size_t LPTIM_INDEX{1};

  using SysTickAccessor =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::SYSTICK_FACILITY, 0>::template accessor<System, PeripheralRegistry>;
  using RccAccessor =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::RCC_FACILITY, 0>::template accessor<System, PeripheralRegistry>;
  using LptimAccessor =  //
      typename PeripheralRegistry::template configuration<
          config::Facility::TIMER_FACILITY, LPTIM_INDEX>::template accessor<System,
                                                                            PeripheralRegistry>;

  Clock<SLEEP_STRATEGY> clock_{static_cast<SysTickAccessor&>(static_cast<System&>(*this)).get(),
                               static_cast<RccAccessor&>(static_cast<System&>(*this)).get(),
                               static_cast<LptimAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] auto& get() noexcept { return clock_; }
  [[nodiscard]] auto& clock0() noexcept { return get(); }
  [[nodiscard]] auto& steady_clock() noexcept { return get(); }
};

}  // namespace tvsc::hal2::clock
