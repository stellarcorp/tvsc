#include "hal/i2c/stm32l4xx_i2c.h"

#include "hal/error.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::i2c {

void I2cStm32l4xx::enable() {
  if (i2c_.Instance == I2C1) {
    __HAL_RCC_I2C1_CLK_ENABLE();
  } else if (i2c_.Instance == I2C2) {
    __HAL_RCC_I2C2_CLK_ENABLE();
  } else if (i2c_.Instance == I2C3) {
    __HAL_RCC_I2C3_CLK_ENABLE();
  } else if (i2c_.Instance == I2C4) {
    __HAL_RCC_I2C4_CLK_ENABLE();
  } else {
    error();
  }

  i2c_.Init.ClockSpeed = 100000;
  i2c_.Init.DutyCycle = I2C_DUTYCYCLE_2;
  i2c_.Init.OwnAddress1 = 0;
  i2c_.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  i2c_.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  i2c_.Init.OwnAddress2 = 0;
  i2c_.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  i2c_.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  require(HAL_I2C_Init(&i2c_) == HAL_OK);
}

void I2cStm32l4xx::disable() {
  HAL_I2C_Deinit(&i2c_);

  if (i2c_.Instance == I2C1) {
    __HAL_RCC_I2C1_CLK_DISABLE();
  } else if (i2c_.Instance == I2C2) {
    __HAL_RCC_I2C2_CLK_DISABLE();
  } else if (i2c_.Instance == I2C3) {
    __HAL_RCC_I2C3_CLK_DISABLE();
  } else if (i2c_.Instance == I2C4) {
    __HAL_RCC_I2C4_CLK_DISABLE();
  } else {
    error();
  }
}

void I2cStm32l4xx::send(uint8_t addr, const uint8_t* data, uint16_t size,
                        std::chrono::milliseconds timeout) {}

void I2cStm32l4xx::receive(uint8_t addr, uint8_t* data, uint16_t size,
                           std::chrono::milliseconds timeout_ms) {}

void I2cStm32l4xx::handle_interrupt() {}

}  // namespace tvsc::hal::i2c
