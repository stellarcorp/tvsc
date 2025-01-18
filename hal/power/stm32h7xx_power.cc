#include "hal/power/stm32h7xx_power.h"

namespace tvsc::hal::power {

void PowerStm32H7xx::enter_low_power_run_mode() {
  // The STM32H7xx series does not have a low power run mode. Power consumption is modulated by
  // changing the clock frequencies or entering sleep or standby modes.
}

void PowerStm32H7xx::exit_low_power_run_mode() {
  // The STM32H7xx series does not have a low power run mode. Power consumption is modulated by
  // changing the clock frequencies or entering sleep or standby modes.
}

}  // namespace tvsc::hal::power
