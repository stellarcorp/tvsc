#pragma once

#include "hal2/peripheral.h"

namespace tvsc::hal2::rcc {

class OscillatorDriver;

class Oscillator : public Peripheral<Oscillator, OscillatorDriver> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

 public:
  virtual ~Oscillator() = default;
};

class OscillatorDriver final : public Driver<Oscillator, OscillatorDriver> {
 private:
  explicit OscillatorDriver(Oscillator& peripheral) noexcept
      : Driver<Oscillator, OscillatorDriver>(peripheral) {}

  friend class Peripheral<Oscillator, OscillatorDriver>;

 public:
  OscillatorDriver() noexcept = default;

  // No direct public methods. All public methods come from superclasses.
};

}  // namespace tvsc::hal2::rcc
