#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

#include "hal/gpio/gpio.h"
#include "hal/i2c/i2c.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::i2c {

class I2cStm32l4xx final : public I2cPeripheral {
 private:
  I2C_HandleTypeDef i2c_{};
  Callback pending_callback_;
  gpio::PinPeripheral scl_pin_peripheral_;
  gpio::PinPeripheral sda_pin_peripheral_;
  gpio::Pin scl_pin_{};
  gpio::Pin sda_pin_{};

  void enable() override;
  void disable() override;

  bool is_device_ready(uint8_t device_address) override;

  // Blocking single-register read
  bool read(uint8_t device_address, uint8_t register_address, uint8_t* data,
            size_t length) override;

  // Blocking single-register write
  bool write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
             size_t length) override;

  // Asynchronous single-register read using DMA
  void read_async(uint8_t device_address, uint8_t register_address, uint8_t* data, size_t length,
                  Callback callback) override;

  // Asynchronous single-register write using DMA
  void write_async(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                   size_t length, Callback callback) override;

  // Blocking FIFO read (optimized bulk transfer without re-sending register address)
  bool read_fifo(uint8_t device_address, uint8_t register_address, uint8_t* data,
                 size_t length) override;

  // Blocking FIFO write (optimized bulk transfer)
  bool write_fifo(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                  size_t length) override;

  // Asynchronous FIFO read using DMA
  void read_fifo_async(uint8_t device_address, uint8_t register_address, uint8_t* data,
                       size_t length, Callback callback) override;

  // Asynchronous FIFO write using DMA
  void write_fifo_async(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                        size_t length, Callback callback) override;

 public:
  I2cStm32l4xx(I2C_TypeDef* i2c_instance, gpio::PinPeripheral scl_pin, gpio::PinPeripheral sda_pin)
      : scl_pin_peripheral_(std::move(scl_pin)), sda_pin_peripheral_(std::move(sda_pin)) {
    i2c_.Instance = i2c_instance;
  }

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::i2c
