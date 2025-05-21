#pragma once

#include "hal/gpio/gpio.h"
#include "hal/programmer/programmer.h"

namespace tvsc::hal::programmer {

class ProgrammerStm32l4xx final : public ProgrammerPeripheral {
 private:
  gpio::GpioPeripheral* gpio_peripheral_;
  gpio::Gpio gpio_{};
  gpio::Pin reset_pin_;
  gpio::Pin boot0_pin_;

  void enable() override;
  void disable() override;

  void initiate_target_reset() override;
  void conclude_target_reset() override;

  void initiate_target_bootloader_boot() override;
  void conclude_target_bootloader_boot() override;

 public:
  ProgrammerStm32l4xx(gpio::GpioPeripheral& gpio_peripheral, gpio::Pin reset_pin,
                      gpio::Pin boot0_pin)
      : gpio_peripheral_(&gpio_peripheral), reset_pin_(reset_pin), boot0_pin_(boot0_pin) {}
};

}  // namespace tvsc::hal::programmer
