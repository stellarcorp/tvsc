#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>

#include "hal/i2c/i2c.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::i2c {

class I2cStm32l4xx final : public I2cPeripheral {
 private:
  I2C_HandleTypeDef i2c_{};

  void enable() override;
  void disable() override;

  void send(uint8_t addr, const uint8_t* data, uint16_t size,
            std::chrono::milliseconds timeout) override;

  void receive(uint8_t addr, uint8_t* data, uint16_t size,
               std::chrono::milliseconds timeout_ms) override;

 public:
  I2cStm32l4xx(I2C_TypeDef* i2c_instance) { i2c_.Instance = i2c_instance; }

  void handle_interrupt() override;
};

}  // namespace tvsc::hal::i2c
