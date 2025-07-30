#pragma once

#include <array>
#include <cstdint>

namespace tvsc::hal::mcu {

using McuId = std::array<uint32_t, 3>;

class Mcu {
 public:
  virtual ~Mcu() = default;

  virtual void read_id(McuId& id) = 0;
};

}  // namespace tvsc::hal::mcu
