#include "hal/timer/timer_noop.h"

namespace tvsc::hal::timer {

PeripheralId TimerNoop::id() { return 0; }

void TimerNoop::start(uint32_t interval_us) {}

bool TimerNoop::is_running() { return false; }

void TimerNoop::stop() {}

void TimerNoop::disable() {}

void TimerNoop::enable() {}

void TimerNoop::handle_interrupt() {}

}  // namespace tvsc::hal::timer
