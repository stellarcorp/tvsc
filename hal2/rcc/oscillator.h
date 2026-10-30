#pragma once

#include "hal2/config/facility.h"
#include "hal2/peripheral.h"

namespace tvsc::hal2::rcc {

class OscillatorDriver;

class Oscillator : public Peripheral<Oscillator, OscillatorDriver> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void connect(config::Facility facility, size_t index) noexcept;

  friend class OscillatorDriver;

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

  void connect(config::Facility facility, size_t index) noexcept {
    peripheral_->connect(facility, index);
  }
};

}  // namespace tvsc::hal2::rcc
