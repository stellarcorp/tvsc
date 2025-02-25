#include "hal/timer/timer_interceptor.h"

namespace tvsc::hal::timer {

PeripheralId TimerInterceptor::id() {
  LOG_FN();
  return call(&InterfaceType::id);
}

void TimerInterceptor::start(uint32_t interval_us) {
  LOG_FN();
  return call(&InterfaceType::start, interval_us);
}

bool TimerInterceptor::is_running() {
  LOG_FN();
  return call(&InterfaceType::is_running);
}

void TimerInterceptor::stop() {
  LOG_FN();
  return call(&InterfaceType::stop);
}

void TimerInterceptor::disable() {
  LOG_FN();
  return call(&InterfaceType::disable);
}

void TimerInterceptor::enable() {
  LOG_FN();
  return call(&InterfaceType::enable);
}

void TimerInterceptor::handle_interrupt() {
  LOG_FN();
  return call(&InterfaceType::handle_interrupt);
}

}  // namespace tvsc::hal::timer
