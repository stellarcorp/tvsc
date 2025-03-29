#pragma once

#include <array>
#include <cstdint>
#include <functional>

#include "hal/peripheral.h"

namespace tvsc::hal::i2c {

static constexpr uint8_t MIN_I2C_ADDRESS{0x08};
static constexpr uint8_t MAX_I2C_ADDRESS{0x77};
static constexpr size_t NUM_VALID_I2C_ADDRESSES{MAX_I2C_ADDRESS - MIN_I2C_ADDRESS + 1};

class I2c;

/**
 * Interface to manage sending and receiving messages over an I2C bus.
 */
class I2cPeripheral : public Peripheral<I2cPeripheral, I2c> {
 public:
  using Callback = std::function<void(bool success)>;

 private:
  virtual void enable() = 0;
  virtual void disable() = 0;

  virtual bool is_device_ready(uint8_t device_address) = 0;

  // Blocking single-register read
  virtual bool read(uint8_t device_address, uint8_t register_address, uint8_t* data,
                    size_t length) = 0;

  // Blocking single-register write
  virtual bool write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                     size_t length) = 0;

  // Asynchronous single-register read using DMA
  virtual void read_async(uint8_t device_address, uint8_t register_address, uint8_t* data,
                          size_t length, Callback callback) = 0;

  // Asynchronous single-register write using DMA
  virtual void write_async(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                           size_t length, Callback callback) = 0;

  // Blocking FIFO read (optimized bulk transfer without re-sending register address)
  virtual bool read_fifo(uint8_t device_address, uint8_t register_address, uint8_t* data,
                         size_t length) = 0;

  // Blocking FIFO write (optimized bulk transfer)
  virtual bool write_fifo(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                          size_t length) = 0;

  // Asynchronous FIFO read using DMA
  virtual void read_fifo_async(uint8_t device_address, uint8_t register_address, uint8_t* data,
                               size_t length, Callback callback) = 0;

  // Asynchronous FIFO write using DMA
  virtual void write_fifo_async(uint8_t device_address, uint8_t register_address,
                                const uint8_t* data, size_t length, Callback callback) = 0;

  friend class I2c;

 public:
  virtual ~I2cPeripheral() = default;

  virtual void handle_interrupt() = 0;
};

class I2c final : public Functional<I2cPeripheral, I2c> {
 public:
  using Callback = I2cPeripheral::Callback;

 private:
  I2c(I2cPeripheral& peripheral) : Functional<I2cPeripheral, I2c>(peripheral) {}

  friend class Peripheral<I2cPeripheral, I2c>;

 public:
  I2c() = default;

  bool is_device_ready(uint8_t device_address) {
    return peripheral_->is_device_ready(device_address);
  }

  // Blocking single-register read
  bool read(uint8_t device_address, uint8_t register_address, uint8_t* data, size_t length) {
    return peripheral_->read(device_address, register_address, data, length);
  }

  // Blocking single-register write
  bool write(uint8_t device_address, uint8_t register_address, const uint8_t* data, size_t length) {
    return peripheral_->write(device_address, register_address, data, length);
  }

  // Asynchronous single-register read using DMA
  void read_async(uint8_t device_address, uint8_t register_address, uint8_t* data, size_t length,
                  Callback callback) {
    peripheral_->read_async(device_address, register_address, data, length, callback);
  }

  // Asynchronous single-register write using DMA
  void write_async(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                   size_t length, Callback callback) {
    peripheral_->write_async(device_address, register_address, data, length, callback);
  }

  // Blocking FIFO read (optimized bulk transfer without re-sending register address)
  bool read_fifo(uint8_t device_address, uint8_t register_address, uint8_t* data, size_t length) {
    return peripheral_->read_fifo(device_address, register_address, data, length);
  }

  // Blocking FIFO write (optimized bulk transfer)
  bool write_fifo(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                  size_t length) {
    return peripheral_->write_fifo(device_address, register_address, data, length);
  }

  // Asynchronous FIFO read using DMA
  void read_fifo_async(uint8_t device_address, uint8_t register_address, uint8_t* data,
                       size_t length, Callback callback) {
    peripheral_->read_fifo_async(device_address, register_address, data, length, callback);
  }

  // Asynchronous FIFO write using DMA
  void write_fifo_async(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                        size_t length, Callback callback) {
    peripheral_->write_fifo_async(device_address, register_address, data, length, callback);
  }
};

template <size_t SIZE>
void scan_bus(I2c& bus, std::array<uint8_t, SIZE>& discovered_devices) {
  size_t index{0};
  for (uint8_t addr = MIN_I2C_ADDRESS; addr <= MAX_I2C_ADDRESS && index < SIZE; ++addr) {
    if (bus.is_device_ready(addr)) {
      discovered_devices[index++] = addr;
    }
  }
}

}  // namespace tvsc::hal::i2c
