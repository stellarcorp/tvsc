#pragma once

#include <cstddef>

namespace tvsc::hal2::config {

enum class Facility {
  // Fundamental processor functionality.
  RCC_FACILITY,
  OSCILLATOR_FACILITY,
  SYSTICK_FACILITY,
  GPIO_FACILITY,
  TIMER_FACILITY,
  WATCHDOG_FACILITY,
  ADC_FACILITY,
  DAC_FACILITY,

  // Buses
  I2C_FACILITY,
  SPI_FACILITY,
  CAN_BUS_FACILITY,

  // Board-level peripherals.
  LED_FACILITY,
  PROGRAMMER_FACILITY,
  IMU_FACILITY,
  POWER_MONITOR_FACILITY,
  RF_RECEIVER_FACILITY,
  RF_TRANSMITTER_FACILITY,
};

struct PeripheralId final {
  Facility facility;
  size_t index;

  constexpr bool operator==(const PeripheralId&) const = default;
};

}  // namespace tvsc::hal2::config
