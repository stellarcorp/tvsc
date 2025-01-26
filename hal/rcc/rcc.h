#pragma once

#include <cstdint>

#include "hal/gpio/gpio.h"

namespace tvsc::hal::rcc {

/**
 * Interface to manage the reset and clock circuitry (RCC) as well as power for select peripherals.
 */
class Rcc {
 public:
  virtual ~Rcc() = default;

  virtual void enable_gpio_port_clock(gpio::Port port) = 0;
  virtual void disable_gpio_port_clock(gpio::Port port) = 0;

  virtual void enable_dac_clock() = 0;
  virtual void disable_dac_clock() = 0;

  virtual void enable_adc_clock() = 0;
  virtual void disable_adc_clock() = 0;

  virtual void set_clock_to_max_speed() = 0;
  virtual void set_clock_to_min_speed() = 0;
};

}  // namespace tvsc::hal::rcc
