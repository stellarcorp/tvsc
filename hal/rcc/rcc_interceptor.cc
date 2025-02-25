#include "hal/rcc/rcc_interceptor.h"

namespace tvsc::hal::rcc {

void RccInterceptor::set_clock_to_max_speed() {
  LOG_FN();
  return call(&InterfaceType::set_clock_to_max_speed);
}

void RccInterceptor::set_clock_to_energy_efficient_speed() {
  LOG_FN();
  return call(&InterfaceType::set_clock_to_energy_efficient_speed);
}

void RccInterceptor::set_clock_to_min_speed() {
  LOG_FN();
  return call(&InterfaceType::set_clock_to_min_speed);
}

void RccInterceptor::restore_clock_speed() {
  LOG_FN();
  return call(&InterfaceType::restore_clock_speed);
}

}  // namespace tvsc::hal::rcc
