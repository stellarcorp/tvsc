#pragma once

#include <cstdint>

namespace tvsc::hal::dac {

class Dac {
 public:
  virtual ~Dac() = default;

  virtual void set_value(uint16_t) = 0;
  virtual void clear_value() = 0;
};

}  // namespace tvsc::hal::dac
