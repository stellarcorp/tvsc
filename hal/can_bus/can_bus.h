#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "hal/peripheral.h"
#include "message/message.h"

namespace tvsc::hal::can_bus {

enum class RxFifo : uint8_t {
  FIFO_ZERO = 0,
  FIFO_ONE = 1,
};

class CanBus;

/**
 * Interface to manage sending and receiving messages over an I2C bus.
 */

// TODO(james): Change the std::array<uint8_t, 8> to a real buffer type, likely a reworked
// tvsc::buffer::Buffer.

class CanBusPeripheral : public Peripheral<CanBusPeripheral, CanBus> {
 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual uint32_t available_message_count(RxFifo fifo) = 0;

  virtual bool receive(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) = 0;

  virtual bool transmit(uint32_t identifier, const std::array<uint8_t, 8>& data) = 0;

  virtual uint32_t error_code() const = 0;

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

  uint32_t available_message_count(RxFifo fifo) {
    return peripheral_->available_message_count(fifo);
  }

  bool receive_raw(RxFifo fifo, uint32_t& identifier, std::array<uint8_t, 8>& data) {
    return peripheral_->receive(fifo, identifier, data);
  }

  bool receive(RxFifo fifo, message::CanBusMessage& message) {
    return peripheral_->receive(fifo, message.identifier(), message.payload());
  }

  bool transmit_raw(uint32_t identifier, const std::array<uint8_t, 8>& data) {
    return peripheral_->transmit(identifier, data);
  }

  bool transmit(uint32_t identifier, const std::string& str) {
    std::array<uint8_t, 8> data{};
    const std::size_t copy_length{std::min(data.size(), str.size())};
    std::copy(str.begin(), str.begin() + copy_length, data.begin());
    return peripheral_->transmit(identifier, data);
  }

  bool transmit(const message::CanBusMessage& message) {
    return peripheral_->transmit(message.identifier(), message.payload());
  }

  uint32_t error_code() const { return peripheral_->error_code(); }
};

}  // namespace tvsc::hal::can_bus
