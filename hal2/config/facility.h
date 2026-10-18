#pragma once

namespace tvsc::hal2::config {

enum class Facility {
  // Fundamental processor functionality.
  RCC_FACILITY,
  OSCILLATOR_FACILITY,
  SYSTICK_FACILITY,
  GPIO_FACILITY,
  LED_FACILITY,
  TIMER_FACILITY,
  WATCHDOG_FACILITY,
  ADC_FACILITY,
  DAC_FACILITY,

  // Buses
  I2C_FACILITY,
  SPI_FACILITY,
  CAN_BUS_FACILITY,

  // Board-level peripherals.
  PROGRAMMER_FACILITY,
  IMU_FACILITY,
  POWER_MONITOR_FACILITY,
  RF_RECEIVER_FACILITY,
  RF_TRANSMITTER_FACILITY,
};

}  // namespace tvsc::hal2::config
