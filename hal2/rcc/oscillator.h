#pragma once

#include "hal2/devices/device.h"

namespace tvsc::hal2::rcc {

class OscillatorDriver;

class Oscillator : public devices::Device<Oscillator, OscillatorDriver> {
 private:
  virtual void enable() noexcept = 0;
  virtual void disable() noexcept = 0;

  virtual void connect(devices::Facility facility, size_t index) noexcept = 0;

  friend class OscillatorDriver;

 public:
  virtual ~Oscillator() = default;
};

class OscillatorDriver final : public devices::Driver<Oscillator, OscillatorDriver> {
 private:
  explicit OscillatorDriver(Oscillator& device) noexcept
      : devices::Driver<Oscillator, OscillatorDriver>(device) {}

  friend class devices::Device<Oscillator, OscillatorDriver>;

 public:
  OscillatorDriver() noexcept = default;

  void connect(devices::Facility facility, size_t index) noexcept { device_->connect(facility, index); }
};

}  // namespace tvsc::hal2::rcc
