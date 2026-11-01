#pragma once

#include "hal2/device.h"
#include "hal2/device.h"

namespace tvsc::hal2::rcc {

class OscillatorDriver;

class Oscillator : public Device<Oscillator, OscillatorDriver> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void connect(Facility facility, size_t index) noexcept;

  friend class OscillatorDriver;

 public:
  virtual ~Oscillator() = default;
};

class OscillatorDriver final : public Driver<Oscillator, OscillatorDriver> {
 private:
  explicit OscillatorDriver(Oscillator& device) noexcept
      : Driver<Oscillator, OscillatorDriver>(device) {}

  friend class Device<Oscillator, OscillatorDriver>;

 public:
  OscillatorDriver() noexcept = default;

  void connect(Facility facility, size_t index) noexcept {
    device_->connect(facility, index);
  }
};

}  // namespace tvsc::hal2::rcc
