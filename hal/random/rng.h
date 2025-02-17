#pragma once

#include <cstdint>

#include "hal/peripheral.h"

namespace tvsc::hal::random {

class Rng;

class RngPeripheral : public Peripheral<RngPeripheral, Rng> {
 private:
  // Turn on power and clock to this peripheral.
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual uint32_t generate() = 0;

  friend class Rng;

 public:
  virtual ~RngPeripheral() = default;
};

class Rng final : public Functional<RngPeripheral, Rng> {
 protected:
  explicit Rng(RngPeripheral& peripheral) : Functional<RngPeripheral, Rng>(peripheral) {}

  friend class Peripheral<RngPeripheral, Rng>;

 public:
  uint32_t operator()() { return peripheral_->generate(); }
};

}  // namespace tvsc::hal::random
