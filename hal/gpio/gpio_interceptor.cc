#include "hal/gpio/gpio_interceptor.h"

namespace tvsc::hal::gpio {

void GpioInterceptor::enable() {
  log_fn(__func__);
  return call(&InterfaceType::enable);
}

void GpioInterceptor::disable() {
  log_fn(__func__);
  return call(&InterfaceType::disable);
}

void GpioInterceptor::set_pin_mode(Pin pin, PinMode mode, PinSpeed speed) {
  log_fn(__func__);
  return call(&InterfaceType::set_pin_mode, pin, mode, speed);
}

bool GpioInterceptor::read_pin(Pin pin) {
  log_fn(__func__);
  return call(&InterfaceType::read_pin, pin);
}

void GpioInterceptor::write_pin(Pin pin, bool on) {
  log_fn(__func__);
  return call(&InterfaceType::write_pin, pin, on);
}

void GpioInterceptor::toggle_pin(Pin pin) {
  log_fn(__func__);
  return call(&InterfaceType::toggle_pin, pin);
}

}  // namespace tvsc::hal::gpio
