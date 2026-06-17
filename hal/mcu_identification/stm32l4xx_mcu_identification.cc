#include "hal/mcu_identification/stm32l4xx_mcu_identification.h"

#include <cstdint>

#include "hal/mcu_identification/mcu_identification.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::mcu_identification {

void McuIdentificationStm32l4xx::read_id(McuId &id) {
  id[0] = *reinterpret_cast<const uint32_t *>(UID_BASE);
  id[1] = *reinterpret_cast<const uint32_t *>(UID_BASE + 4);
  id[2] = *reinterpret_cast<const uint32_t *>(UID_BASE + 8);
}

}  // namespace tvsc::hal::mcu_identification
