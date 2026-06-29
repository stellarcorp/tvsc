#pragma once

#include "hal/peripheral.h"

namespace tvsc::hal::led {

class Led;

class LedPeripheral : public Peripheral<LedPeripheral, Led> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void on() = 0;
  virtual void off() = 0;
  virtual void toggle() = 0;
  virtual bool read() = 0;

  friend class Led;

 public:
  virtual ~LedPeripheral() = default;
};

class Led final : public Functional<LedPeripheral, Led> {
 private:
  Led(LedPeripheral& peripheral) : Functional<LedPeripheral, Led>(peripheral) {}

  friend class Peripheral<LedPeripheral, Led>;

 public:
  Led() = default;

  void on() { return peripheral_->on(); }
  void off() { return peripheral_->off(); }
  void toggle() { return peripheral_->toggle(); }
  bool read() { return peripheral_->read(); }
};

}  // namespace tvsc::hal::led
