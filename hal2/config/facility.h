#pragma once

#include <cstddef>

namespace tvsc::hal2::config {

enum class Facility {
  // Fundamental processor functionality.
  RCC_FACILITY,  // Note that RCC includes the primary oscillator as well.

  // Time-oriented facilities.
  OSCILLATOR_FACILITY,  // Secondary oscillators.
  SYSTICK_FACILITY,
  TIMER_FACILITY,
  CLOCK_FACILITY,

  // Random number generation.
  RNG_FACILITY,

  // Interactions via the pins of the processor.
  GPIO_FACILITY,

  // Watchdogs to verify behavior.
  WATCHDOG_FACILITY,

  // Analog-digital conversion.
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
