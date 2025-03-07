#pragma once

#include <chrono>

#include "hal/gpio/gpio.h"
#include "hal/simulation/interceptor.h"
#include "hal/peripheral.h"

namespace tvsc::hal::gpio {

template <typename ClockType>
class GpioInterceptor final : public simulation::Interceptor<GpioPeripheral, ClockType> {
 public:
  explicit GpioInterceptor(GpioPeripheral& gpio) : simulation::Interceptor<GpioPeripheral, ClockType>(gpio) {}

  void enable() override {
    LOG_FN();
    return this->call(&GpioPeripheral::enable);
  }

  void disable() override {
    LOG_FN();
    return this->call(&GpioPeripheral::disable);
  }

  void set_pin_mode(Pin pin, PinMode mode, PinSpeed speed) override {
    LOG_FN();
    return this->call(&GpioPeripheral::set_pin_mode, pin, mode, speed);
  }

  bool read_pin(Pin pin) override {
    LOG_FN();
    return this->call(&GpioPeripheral::read_pin, pin);
  }

  void write_pin(Pin pin, bool on) override {
    LOG_FN();
    return this->call(&GpioPeripheral::write_pin, pin, on);
  }

  void toggle_pin(Pin pin) override {
    LOG_FN();
    return this->call(&GpioPeripheral::toggle_pin, pin);
  }
};

}  // namespace tvsc::hal::gpio
