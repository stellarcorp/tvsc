#include "hal/watchdog/watchdog_interceptor.h"

namespace tvsc::hal::watchdog {

void WatchdogInterceptor::enable() {
  LOG_FN();
  return call(&InterfaceType::enable);
}

void WatchdogInterceptor::disable() {
  LOG_FN();
  return call(&InterfaceType::disable);
}

void WatchdogInterceptor::feed() {
  LOG_FN();
  return call(&InterfaceType::feed);
}

[[nodiscard]] std::chrono::milliseconds WatchdogInterceptor::reset_interval() {
  LOG_FN();
  return call(&InterfaceType::reset_interval);
}

}  // namespace tvsc::hal::watchdog
