#pragma once

#include "hal/mcu/mcu.h"

namespace tvsc::hal::mcu {

class McuStm32l4xx final : public Mcu {
 public:
  void read_id(McuId& id) override;
};

}  // namespace tvsc::hal::mcu
