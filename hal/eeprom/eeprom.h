#pragma once

#include <stdexcept>

#include "base/except.h"

namespace tvsc::hal::eeprom {

class Eeprom final {
 public:
  /**
   * Some platforms require initialization of the EEPROM. When initialization is required, it should
   * be implemented in this constructor. This also means that the methods below shouldn't be static,
   * since they may require initialization; by using instance methods, that initialization is
   * guaranteed.
   */
  Eeprom();
  ~Eeprom();

  uint8_t read(size_t index) const;
  void read(size_t index, void* dest, size_t length) const;

  void write(size_t index, uint8_t value);
  void write(size_t index, const void* src, size_t length);

  bool is_empty(size_t index) const;

  size_t length() const;

  template <typename T>
  void get(size_t index, T& value) const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Eeprom.get() is only usable on types that are trivially copyable");
    if (index + sizeof(T) >= length()) {
      except<std::out_of_range>("Index out of range in Eeprom::get()");
    }
    read(index, reinterpret_cast<void*>(&value), sizeof(T));
  }

  /**
   * Put a value into the EEPROM at the given index.
   *
   * EEPROMs tend to have write limits -- they degrade as they are written to. Usually, this
   * degradation becomes apparent around 100,000 writes or so, with each EEPROM having different
   * characteristics.
   *
   * EEPROMs can be slow to write to. It can take multiple milliseconds to do a single write.
   */
  template <typename T>
  void put(size_t index, const T& value) {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Eeprom.put() is only usable on types that are trivially copyable");
    if (index + sizeof(T) >= length()) {
      except<std::out_of_range>("Index out of range in Eeprom::put()");
    }
    write(index, reinterpret_cast<const void*>(&value), sizeof(T));
  }

  /**
   * Write the value to the EEPROM at the given index only if that value differs from the current
   * contents.
   *
   * This method avoids degrading the EEPROM unnecessarily. In most cases, this is the method you
   * should use to write to the EEPROM. EEPROM writes are already very slow. The extra slowdown of
   * this method will not be noticeable in most use cases.
   */
  template <typename T>
  void update(size_t index, const T& value) {
    if (index + sizeof(T) >= length()) {
      except<std::out_of_range>("Index out of range in Eeprom::get()");
    }
    for (size_t i = 0; i < sizeof(T); ++i) {
      uint8_t val = read(index + i);
      if (*(reinterpret_cast<const uint8_t*>(value) + i) != val) {
        put(index, value);
        return;
      }
    }
  }
};
}  // namespace tvsc::hal::eeprom
