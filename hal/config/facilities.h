#pragma once

namespace tvsc::hal::config {

enum class Facility {
  // Fundamental processor functionality.
  RCC,
  SYSTICK,
  POWER,
  GPIO,
  LED,
  TIMER,
  WATCHDOG,
  ADC,
  DAC,

  // Buses
  I2C,
  SPI,
  CAN_BUS,

  // Board-level peripherals.
  PROGRAMMER,
  IMU,
  POWER_MONITOR,
  RF_RECEIVER,
  RF_TRANSMITTER,
};

}  // namespace tvsc::hal::config
