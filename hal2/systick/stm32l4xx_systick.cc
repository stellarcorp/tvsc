#include "hal2/systick/stm32l4xx_systick.h"

#include "hal2/systick/stm32l4_systick_config.h"
#include "hal2/systick/systick.h"
#include "hal2/time_type.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::systick {

SysTickStm32l4xx::SysTickStm32l4xx() noexcept {
}

[[nodiscard]] TimeType SysTickStm32l4xx::current_time_micros() const noexcept { return uwTick; }

void SysTickStm32l4xx::increment_micros(TimeType us) noexcept { uwTick += us; }

void handle_systick_interrupt() noexcept {
  // SysTick fires every millisecond. We keep the tick counter in microseconds in order to better
  // avoid round-off errors.
  uwTick += 1000;
}

}  // namespace tvsc::hal2::systick
