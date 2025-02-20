#include "hal/power/stm32l4xx_power.h"

#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::power {

void PowerStm32L4xx::enter_stop_mode() { HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI); }

}  // namespace tvsc::hal::power
