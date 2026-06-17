#pragma once

#include "hal/mcu_identification/mcu_identification.h"

namespace tvsc::hal::mcu_identification {

class McuIdentificationStm32l4xx final : public McuIdentification {
 public:
  void read_id(McuId& id) override;
};

}  // namespace tvsc::hal::mcu_identification
