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

  virtual void set_clock_to_max_speed() = 0;
  virtual void set_clock_to_min_speed() = 0;
};

/**
 * Manage the oscillator for USB, RNG, and other functions that require a precise, (relatively) high
 * speed oscillator.
 *
 * In the STM32L4xx series, this oscillator can only be used as the clock for USB and RNG. It cannot
 * be used as a generic SYSCLK or drive any other peripherals. This limitation makes it ideal for
 * clocking those two peripherals. We do not need to disturb the SYSCLK or the clocks of other
 * peripherals.
 *
 * This oscillator is likely specific to STM32 processors, but it's unclear how this functionality
 * could be abstracted better at this time. Somehow, we would need to include the notion of multiple
 * oscillators, model which oscillators can provide clock signals for which sets of peripherals, and
 * manage routing those oscillators dynamically. Rather than tackle this problem, we have punted on
 * it completely and simply named this oscillator as it is named in the STM32 MCU documentation.
 */
class Hsi48Activation;

class Hsi48Oscillator : public Peripheral<Hsi48Oscillator, Hsi48Activation> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

 public:
  virtual ~Hsi48Oscillator() = default;
};

class Hsi48Activation final : public Functional<Hsi48Oscillator, Hsi48Activation> {
 private:
  // Existence of a valid instance of this class on the stack ensures that the HSI 48 clock is
  // enabled. The clock is disabled when all of these instances fall out of scope (RAII) or are
  // invalidate()'d.

  explicit Hsi48Activation(Hsi48Oscillator& peripheral)
      : Functional<Hsi48Oscillator, Hsi48Activation>(peripheral) {}

  friend class Peripheral<Hsi48Oscillator, Hsi48Activation>;

 public:
  Hsi48Activation() = default;

  // No direct public methods. Any public methods come from superclasses.
};

}  // namespace tvsc::hal::rcc
