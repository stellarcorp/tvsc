#include "hal/systick/stm32l4xx_systick.h"

#include "hal/systick/systick.h"
#include "hal/time_type.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::systick {

TimeType SysTickStm32l4xx::current_time_micros() { return uwTick; }

void SysTickStm32l4xx::increment_micros(TimeType us) { uwTick += us; }

void SysTickStm32l4xx::handle_interrupt() {
  // SysTick fires every millisecond. We keep the tick counter in microseconds in order to better
  // avoid round-off errors.
  uwTick += 1000;
}

}  // namespace tvsc::hal::systick
