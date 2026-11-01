#pragma once

#include "hal2/clock/clock.h"
#include "hal2/config/peripheral_definitions.h"

namespace tvsc::hal2::clock {

struct ClockConfiguration final {
  clock::SleepStrategy sleep_strategy{clock::SleepStrategy::SLEEP};
};

template <typename System, typename PeripheralRegistry, size_t INDEX, ClockConfiguration CONFIG>
  requires(INDEX == 0)
class ClockAccessor {
 private:
  static constexpr size_t LPTIM_INDEX{1};

  using SysTickAccessor =
      config::accessor_type<System, PeripheralRegistry, {Facility::SYSTICK_FACILITY, 0}>;
  using RccAccessor =
      config::accessor_type<System, PeripheralRegistry, {Facility::RCC_FACILITY, 0}>;
  using LptimAccessor =
      config::accessor_type<System, PeripheralRegistry, {Facility::TIMER_FACILITY, LPTIM_INDEX}>;

  Clock<CONFIG.sleep_strategy> clock_{
      static_cast<SysTickAccessor&>(static_cast<System&>(*this)).get(),
      static_cast<RccAccessor&>(static_cast<System&>(*this)).get(),
      static_cast<LptimAccessor&>(static_cast<System&>(*this)).get()};

 public:
  [[nodiscard]] auto& get() noexcept { return clock_; }
  [[nodiscard]] auto& clock0() noexcept { return get(); }
  [[nodiscard]] auto& steady_clock() noexcept { return get(); }
};

}  // namespace tvsc::hal2::clock
