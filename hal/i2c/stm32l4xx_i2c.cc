#include "hal/i2c/stm32l4xx_i2c.h"

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

extern "C" {

__attribute__((section(".status.value"))) uint8_t i2c_enabled{};

}  // extern "C"

namespace tvsc::hal::i2c {

void I2cStm32l4xx::enable() {
  using namespace tvsc::hal::gpio;

  gpio_ = gpio_peripheral_->access();
  gpio_.set_pin_mode(scl_pin_, PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP,
                     PinSpeed::VERY_HIGH, GPIO_AF4_I2C1);
  gpio_.set_pin_mode(sda_pin_, PinMode::ALTERNATE_FUNCTION_OPEN_DRAIN_WITH_PULL_UP,
                     PinSpeed::VERY_HIGH, GPIO_AF4_I2C1);

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

  // The API for specifying the timing field depends on the clock speed for the I2C's bus. This
  // value was derived for 100 kHz signals with a bus speed of 16 MHz. There is no API to calculate
  // this value.
  // i2c_.Init.Timing = 0x00702991;
  // 400 kHz with a bus speed of 16 MHz.
  i2c_.Init.Timing = 0x00300F33;
  // 400 kHz with a bus speed of 4 MHz.
  // i2c_.Init.Timing = 0x00310309;

  i2c_.Init.OwnAddress1 = 0;
  i2c_.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  i2c_.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  i2c_.Init.OwnAddress2 = 0;
  i2c_.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  i2c_.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  require(HAL_I2C_Init(&i2c_) == HAL_OK);

  i2c_enabled = 1;
}

void I2cStm32l4xx::disable() {
  HAL_I2C_DeInit(&i2c_);

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

  gpio_.invalidate();
}

bool I2cStm32l4xx::is_device_ready(uint8_t device_address) {
  return HAL_I2C_IsDeviceReady(&i2c_, (device_address << 1), /* trials */ 10, 1000) == HAL_OK;
}

// Blocking single-register read
bool I2cStm32l4xx::read(uint8_t device_address, uint8_t register_address, uint8_t* data,
                        size_t length) {
  return HAL_I2C_Mem_Read(&i2c_, device_address << 1, register_address, I2C_MEMADD_SIZE_8BIT, data,
                          length, 1000) == HAL_OK;
}

// Blocking single-register write
bool I2cStm32l4xx::write(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                         size_t length) {
  return HAL_I2C_Mem_Write(&i2c_, device_address << 1, register_address, I2C_MEMADD_SIZE_8BIT,
                           const_cast<uint8_t*>(data), length, 1000) == HAL_OK;
}

// Asynchronous single-register read using DMA
void I2cStm32l4xx::read_async(uint8_t device_address, uint8_t register_address, uint8_t* data,
                              size_t length, Callback callback) {
  pending_callback_ = callback;
  if (HAL_I2C_Mem_Read_DMA(&i2c_, device_address << 1, register_address, I2C_MEMADD_SIZE_8BIT, data,
                           length) != HAL_OK) {
    if (callback) callback(false);
  }
}

// Asynchronous single-register write using DMA
void I2cStm32l4xx::write_async(uint8_t device_address, uint8_t register_address,
                               const uint8_t* data, size_t length, Callback callback) {
  pending_callback_ = callback;
  if (HAL_I2C_Mem_Write_DMA(&i2c_, device_address << 1, register_address, I2C_MEMADD_SIZE_8BIT,
                            const_cast<uint8_t*>(data), length) != HAL_OK) {
    if (callback) callback(false);
  }
}

// Blocking FIFO read (optimized bulk transfer without re-sending register address)
bool I2cStm32l4xx::read_fifo(uint8_t device_address, uint8_t register_address, uint8_t* data,
                             size_t length) {
  if (HAL_I2C_Master_Transmit(&i2c_, device_address << 1, &register_address, 1, 1000) != HAL_OK) {
    return false;  // Failed to write FIFO register address
  }
  return (HAL_I2C_Master_Receive(&i2c_, device_address << 1, data, length, 1000) == HAL_OK);
}

// Blocking FIFO write (optimized bulk transfer)
bool I2cStm32l4xx::write_fifo(uint8_t device_address, uint8_t register_address, const uint8_t* data,
                              size_t length) {
  return HAL_I2C_Master_Transmit(&i2c_, device_address << 1, const_cast<uint8_t*>(data), length,
                                 1000) == HAL_OK;
}

// Asynchronous FIFO read using DMA
void I2cStm32l4xx::read_fifo_async(uint8_t device_address, uint8_t register_address, uint8_t* data,
                                   size_t length, Callback callback) {
  if (HAL_I2C_Master_Transmit(&i2c_, device_address << 1, &register_address, 1, 1000) == HAL_OK) {
    if (HAL_I2C_Master_Receive_DMA(&i2c_, device_address << 1, data, length) == HAL_OK) {
      return;
    }
  }
  // If either of the HAL calls fail, we call the callback.
  callback(false);
}

// Asynchronous FIFO write using DMA
void I2cStm32l4xx::write_fifo_async(uint8_t device_address, uint8_t register_address,
                                    const uint8_t* data, size_t length, Callback callback) {
  pending_callback_ = callback;
  if (HAL_I2C_Master_Transmit_DMA(&i2c_, device_address << 1, const_cast<uint8_t*>(data), length) !=
      HAL_OK) {
    if (callback) callback(false);
  }
}

void I2cStm32l4xx::handle_interrupt() {
  if (pending_callback_) {
    pending_callback_(true);
  }
}

}  // namespace tvsc::hal::i2c
