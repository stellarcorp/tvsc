#pragma once

#include "hal/gpio/gpio.h"
#include "hal/led/led.h"

namespace tvsc::hal::led {

class HalLed final : public LedPeripheral {
 private:
  gpio::PinPeripheral led_peripheral_;
  gpio::Pin led_{};

  void enable() override;
  void disable() override;

  void on() override;
  void off() override;
  void toggle() override;
  bool read() override;

 public:
  HalLed(gpio::PinPeripheral led_peripheral) : led_peripheral_(std::move(led_peripheral)) {}
};

}  // namespace tvsc::hal::led
