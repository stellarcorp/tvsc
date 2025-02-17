#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include "hal/peripheral.h"

namespace tvsc::hal::dac {

class Dac;

class DacPeripheral : public Peripheral<DacPeripheral, Dac> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual void set_value(uint32_t value, uint8_t channel) = 0;
  virtual void clear_value(uint8_t channel) = 0;

  virtual void set_resolution(uint8_t bits_resolution, uint8_t channel) = 0;

  friend class Dac;

 public:
  virtual ~DacPeripheral() = default;
};

class Dac final : public Functional<DacPeripheral, Dac> {
 private:
  explicit Dac(DacPeripheral& peripheral) : Functional<DacPeripheral, Dac>(peripheral) {}

  friend class Peripheral<DacPeripheral, Dac>;

 public:
  void set_value(uint32_t value, uint8_t channel = 0) { peripheral_->set_value(value, channel); }
  void clear_value(uint8_t channel = 0) { peripheral_->clear_value(channel); }

  void set_resolution(uint8_t bits_resolution, uint8_t channel = 0) {
    peripheral_->set_resolution(bits_resolution, channel);
  }
};

}  // namespace tvsc::hal::dac
