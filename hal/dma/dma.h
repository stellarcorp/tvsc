#pragma once

#include <cstdint>

#include "hal/peripheral.h"
#include "hal/peripheral_id.h"

namespace tvsc::hal::dma {

class Dma;

class DmaPeripheral : public Peripheral<DmaPeripheral, Dma> {
 private:
  // Turn on power and clock to this peripheral.
  virtual void enable() = 0;
  virtual void disable() = 0;

  // Peripheral id for linking peripherals together in a platform-dependent manner.
  virtual PeripheralId id() = 0;

  virtual void start_circular_transfer() = 0;

  friend class Dma;

 public:
  virtual ~DmaPeripheral() = default;

  virtual void handle_interrupt() = 0;
};

class Dma final : public Functional<DmaPeripheral, Dma> {
 protected:
  explicit Dma(DmaPeripheral& peripheral) : Functional<DmaPeripheral, Dma>(peripheral) {}

  friend class Peripheral<DmaPeripheral, Dma>;

 public:
  Dma() = default;

  PeripheralId id() { return peripheral_->id(); }

  void start_circular_transfer() { peripheral_->start_circular_transfer(); }
};

}  // namespace tvsc::hal::dma
