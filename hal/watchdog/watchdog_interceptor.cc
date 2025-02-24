#include "hal/watchdog/watchdog_interceptor.h"

namespace tvsc::hal::watchdog {

void WatchdogInterceptor::enable() {
  log_fn(__func__);
  return call(&InterfaceType::enable);
}

void WatchdogInterceptor::disable() {
  log_fn(__func__);
  return call(&InterfaceType::disable);
}

void WatchdogInterceptor::feed() {
  log_fn(__func__);
  return call(&InterfaceType::feed);
}

[[nodiscard]] std::chrono::milliseconds WatchdogInterceptor::reset_interval() {
  log_fn(__func__);
  return call(&InterfaceType::reset_interval);
}

}  // namespace tvsc::hal::watchdog
