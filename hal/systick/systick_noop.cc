#include "hal/systick/systick_noop.h"

#include "hal/systick/systick.h"
#include "hal/time_type.h"

extern "C" {
volatile tvsc::hal::TimeType uwTick{};
}

namespace tvsc::hal::systick {

TimeType SysTickNoop::current_time_micros() { return uwTick; }

void SysTickNoop::increment_micros(TimeType us) { uwTick += us; }

void SysTickNoop::handle_interrupt() { uwTick += 1000; }

}  // namespace tvsc::hal::systick
