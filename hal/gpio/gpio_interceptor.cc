#include "hal/gpio/gpio_interceptor.h"

namespace tvsc::hal::gpio {

void GpioInterceptor::enable() {
  LOG_FN();
  return call(&InterfaceType::enable);
}

void GpioInterceptor::disable() {
  LOG_FN();
  return call(&InterfaceType::disable);
}

void GpioInterceptor::set_pin_mode(Pin pin, PinMode mode, PinSpeed speed) {
  LOG_FN();
  return call(&InterfaceType::set_pin_mode, pin, mode, speed);
}

bool GpioInterceptor::read_pin(Pin pin) {
  LOG_FN();
  return call(&InterfaceType::read_pin, pin);
}

void GpioInterceptor::write_pin(Pin pin, bool on) {
  LOG_FN();
  return call(&InterfaceType::write_pin, pin, on);
}

void GpioInterceptor::toggle_pin(Pin pin) {
  LOG_FN();
  return call(&InterfaceType::toggle_pin, pin);
}

}  // namespace tvsc::hal::gpio
