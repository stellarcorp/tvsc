#pragma once

#include "hal/gpio/gpio.h"
#include "hal/peripheral.h"
#include "hal/simulation/interceptor.h"
#include "hal/simulation/logger.h"

namespace tvsc::hal::gpio {

template <typename ClockType>
class GpioInterceptor final : public simulation::Interceptor<GpioPeripheral, ClockType> {
 public:
  explicit GpioInterceptor(GpioPeripheral& gpio, simulation::Logger<ClockType>& logger)
      : simulation::Interceptor<GpioPeripheral, ClockType>(gpio, logger) {}

  void enable() override {
    LOG_FN();
    return this->call(&GpioPeripheral::enable);
  }

  void disable() override {
    LOG_FN();
    return this->call(&GpioPeripheral::disable);
  }

  void set_pin_mode(PinNumber pin, PinMode mode, PinSpeed speed,
                    uint8_t alternate_function_mapping) override {
    LOG_FN();
    return this->call(&GpioPeripheral::set_pin_mode, pin, mode, speed, alternate_function_mapping);
  }

  bool read_pin(PinNumber pin) override {
    LOG_FN();
    return this->call(&GpioPeripheral::read_pin, pin);
  }

  void write_pin(PinNumber pin, bool on) override {
    LOG_FN();
    return this->call(&GpioPeripheral::write_pin, pin, on);
  }

  void toggle_pin(PinNumber pin) override {
    LOG_FN();
    return this->call(&GpioPeripheral::toggle_pin, pin);
  }

  PortNumber port() const override {
    LOG_FN();
    return this->call(&GpioPeripheral::port);
  }
};

}  // namespace tvsc::hal::gpio
