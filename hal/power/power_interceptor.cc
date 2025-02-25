#include "hal/power/power_interceptor.h"

namespace tvsc::hal::power {

void PowerInterceptor::enter_sleep_mode() {
  LOG_FN();
  return call(&InterfaceType::enter_sleep_mode);
}

void PowerInterceptor::enter_stop_mode() {
  LOG_FN();
  return call(&InterfaceType::enter_stop_mode);
}

}  // namespace tvsc::hal::power
