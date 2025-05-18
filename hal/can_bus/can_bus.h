#pragma once

#include <array>
#include <cstdint>

#include "hal/peripheral.h"

namespace tvsc::hal::can_bus {

enum class TxMailbox : uint8_t {
  ONE,
  TWO,
  THREE,
};

enum class RxFifo : uint8_t {
  ONE,
  TWO,
  THREE,
};

class CanBus;

/**
 * Interface to manage sending and receiving messages over an I2C bus.
 */
class CanBusPeripheral : public Peripheral<CanBusPeripheral, CanBus> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual bool data_available(RxFifo fifo) = 0;

  virtual void transmit(TxMailbox mailbox, uint32_t identifier,
                        const std::array<uint8_t, 8>& data) = 0;

  virtual void receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) = 0;

  friend class CanBus;

 public:
  virtual ~CanBusPeripheral() = default;

  virtual void handle_interrupt() = 0;
};

class CanBus final : public Functional<CanBusPeripheral, CanBus> {
 private:
  CanBus(CanBusPeripheral& peripheral) : Functional<CanBusPeripheral, CanBus>(peripheral) {}

  friend class Peripheral<CanBusPeripheral, CanBus>;

 public:
  CanBus() = default;

  bool data_available(RxFifo fifo) { return peripheral_->data_available(fifo); }

  void transmit(TxMailbox mailbox, uint32_t identifier, const std::array<uint8_t, 8>& data) {
    return peripheral_->transmit(mailbox, identifier, data);
  }

  void receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) {
    return peripheral_->receive(fifo, identifier, data);
  }
};

}  // namespace tvsc::hal::can_bus
