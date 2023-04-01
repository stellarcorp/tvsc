#include "Arduino.h"
#include "SPI.h"
#include "bus/gpio/pins.h"
#include "bus/gpio/time.h"
#include "bus/spi/spi.h"

#ifndef ATOMIC_BLOCK_START
#define ATOMIC_BLOCK_START
#define ATOMIC_BLOCK_END
#endif

namespace tvsc::bus::spi {

// This file depends heavily on the functions in the gpio namespace.
using namespace tvsc::bus::gpio;

inline SPIClass* as_spi(void* ptr) { return reinterpret_cast<SPIClass*>(ptr); }

SpiBus::SpiBus(void* spi_implementation) : spi_(spi_implementation) {}
SpiBus::~SpiBus() { as_spi(spi_)->end(); }
void SpiBus::init() { as_spi(spi_)->begin(); }

void SpiBus::initialize_peripheral(uint8_t peripheral_select_pin) {
  set_mode(peripheral_select_pin, PinMode::MODE_OUTPUT);

  // Initialize the pin to deselect state.
  write_pin(peripheral_select_pin, DigitalValue::VALUE_HIGH);

  // We get spurious failures without a delay. But this seems excessive.
  // TODO(james): Test to find what delay, if any, is needed. This value is just the first thing I
  // tried that worked.
  delay_ms(100);
}

void SpiBus::using_interrupt(uint8_t interrupt_number) {
  as_spi(spi_)->usingInterrupt(interrupt_number);
}

SpiTransaction SpiBus::begin_transaction(uint8_t peripheral_select_pin) {
  // TODO(james): Change the API to allow these settings to be different for each peripheral.
  static const SPISettings SETTINGS{4'000'000, MSBFIRST, SPI_MODE0};
  ATOMIC_BLOCK_START;
  as_spi(spi_)->beginTransaction(SETTINGS);

  // Select the peripheral by dropping the peripheral's chipselect pin to LOW.
  write_pin(peripheral_select_pin, DigitalValue::VALUE_LOW);

  return SpiTransaction{*this, peripheral_select_pin};
}

void SpiBus::end_transaction(uint8_t peripheral_select_pin) {
  // Based on https://forum.pjrc.com/attachment.php?attachmentid=10948&d=1499109224
  // Note that since the Teensy can run upwards of 900MHz if overclocked, other devices on the bus
  // may not be able to keep up. In particular, there are reports of errors where it appears the
  // Teensy is not holding NSS low long enough for the peripherals to complete their side of the
  // transfer. A full microsecond seems excessive though.
  // TODO(james): Test if this is needed for the radios for the Teensy 4.x at the speeds we use.
  delay_us(1);

  // Deselect the peripheral by raising the peripheral's chipselect pin to HIGH.
  write_pin(peripheral_select_pin, DigitalValue::VALUE_HIGH);

  as_spi(spi_)->endTransaction();
  ATOMIC_BLOCK_END;
}

uint8_t SpiBus::transfer(uint8_t value) { return as_spi(spi_)->transfer(value); }

SpiBus get_default_spi_bus() { return SpiBus(reinterpret_cast<void*>(&SPI)); }

}  // namespace tvsc::bus::spi
