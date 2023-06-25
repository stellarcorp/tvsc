#include "avr/eeprom.h"
#include "avr_functions.h"
#include "hal/eeprom/eeprom.h"

namespace tvsc::hal::eeprom {

Eeprom::Eeprom() { eeprom_initialize(); }

Eeprom::~Eeprom(){
    // Do nothing.
};

uint8_t Eeprom::read(size_t index) const {
  eeprom_busy_wait();
  return eeprom_read_byte(reinterpret_cast<uint8_t*>(index));
}

void Eeprom::read(size_t index, void* dest, size_t length) const {
  eeprom_busy_wait();
  eeprom_read_block(dest, reinterpret_cast<const void*>(index), length);
}

void Eeprom::write(size_t index, uint8_t value) {
  eeprom_busy_wait();
  eeprom_write_byte(reinterpret_cast<uint8_t*>(index), value);
  eeprom_busy_wait();
}

void Eeprom::write(size_t index, const void* src, size_t length) {
  eeprom_busy_wait();
  eeprom_write_block(src, reinterpret_cast<void*>(index), length);
  eeprom_busy_wait();
}

bool Eeprom::is_empty(size_t index) const {
  static constexpr uint8_t NEVER_WRITTEN_VALUE{255};
  return read(index) == NEVER_WRITTEN_VALUE;
}

size_t Eeprom::length() const { return E2END + 1; }

}  // namespace tvsc::hal::eeprom
