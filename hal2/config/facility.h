#pragma once

namespace tvsc::hal2::config {

enum class Facility {
  // Fundamental processor functionality.
  RCC,
  OSCILLATOR,
  SYSTICK,
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

}  // namespace tvsc::hal2::config
