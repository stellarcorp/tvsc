#pragma once

#include "hal2/clock/clock.h"
#include "hal2/clock/hal_clock.h"
#include "hal2/devices/device_definitions.h"

namespace tvsc::hal2::clock {

struct HalClockConfiguration final {
  clock::SleepStrategy sleep_strategy{clock::SleepStrategy::SLEEP};
};

template <typename DeviceRegistry, typename Board, size_t INDEX, HalClockConfiguration CONFIG>
  requires(INDEX == 0)
class HalClockAccessor {
 private:
  static constexpr size_t LPTIM_INDEX{1};

  using SysTickAccessor =
      devices::accessor_type<DeviceRegistry, Board, {devices::Facility::SYSTICK_FACILITY, 0}>;
  using RccAccessor =
      devices::accessor_type<DeviceRegistry, Board, {devices::Facility::RCC_FACILITY, 0}>;
  using LptimAccessor = devices::accessor_type<DeviceRegistry, Board,
                                               {devices::Facility::TIMER_FACILITY, LPTIM_INDEX}>;

  HalClock<CONFIG.sleep_strategy> clock_{
      static_cast<SysTickAccessor&>(static_cast<DeviceRegistry&>(*this)).get(),
      static_cast<RccAccessor&>(static_cast<DeviceRegistry&>(*this)).get(),
      static_cast<LptimAccessor&>(static_cast<DeviceRegistry&>(*this)).get()};

 public:
  [[nodiscard]] Clock& get() noexcept { return clock_; }
  [[nodiscard]] auto& clock0() noexcept { return get(); }
  [[nodiscard]] auto& steady_clock() noexcept { return get(); }
};

}  // namespace tvsc::hal2::clock
