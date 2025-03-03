#include "hal/systick/systick_noop.h"

#include "hal/systick/systick.h"

extern "C" {
volatile uint64_t uwTick{};
}

namespace tvsc::hal::systick {

TimeType SysTickNoop::current_time_micros() { return uwTick; }

void SysTickNoop::increment_micros(TimeType us) { uwTick += us; }

void SysTickNoop::handle_interrupt() { uwTick += 1000; }

}  // namespace tvsc::hal::systick
