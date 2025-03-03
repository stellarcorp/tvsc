#include "hal/systick/systick_interceptor.h"

#include "hal/systick/systick.h"

namespace tvsc::hal::systick {

TimeType SysTickInterceptor::current_time_micros() {
  LOG_FN();
  return call(&InterfaceType::current_time_micros);
}

void SysTickInterceptor::increment_micros(TimeType us) {
  LOG_FN();
  return call(&InterfaceType::increment_micros, us);
}

void SysTickInterceptor::handle_interrupt() {
  LOG_FN();
  return call(&InterfaceType::handle_interrupt);
}

}  // namespace tvsc::hal::systick
