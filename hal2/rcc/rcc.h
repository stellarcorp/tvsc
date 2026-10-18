#pragma once

#include "hal2/peripheral.h"

namespace tvsc::hal2::rcc {

/**
 * Interface to manage the reset and clock circuitry (RCC) as well as run/sleep/stop modes.
 */
class Rcc {
 public:
  virtual ~Rcc() = default;

  virtual void set_clock_to_min_speed() noexcept = 0;
  virtual void set_clock_to_max_speed() noexcept = 0;
  virtual void set_clock_to_energy_efficient_speed() noexcept = 0;

  // Note: be sure to configure an interrupt mechanism before calling these methods. They do not
  // return until an interrupt fires.
  virtual void enter_sleep_mode() noexcept = 0;
  virtual void enter_stop_mode() noexcept = 0;

  // Restore the clock speed to the last requested setting after waking from stop mode. When exiting
  // stop mode, the clock defaults to a 4 MHz MSI clock or the 16 MHz HSI, possibly with no PLL
  // setting. This method is a shorthand to restore the previous clock setting.
  virtual void restore_clock_speed() noexcept = 0;
};

}  // namespace tvsc::hal2::rcc
