#pragma once

#include <array>
#include <cstdint>

#include "hal/time/clock.h"
#include "hal/time/time.h"

extern "C" {

#include "stm32h7xx.h"

/**
 * These symbols are required by the ARM CMSIS code. They are typically defined in a file named
 * system_<device>.c. We declare them here so that they can be used in our code as well; if we're
 * paying for them, we're using them.
 *
 * It's not clear what is required of these symbols or why they are left to user code to implement
 * and manage. I suspect they were originally intended to be hooks to allow for configurability,
 * like the main() function, but instead, they became requirements of the interface. With that
 * mindset, these functions should be assumed to be the most minimal implementation that makes the
 * CPU work, and all other functionality should be available via other abstractions.
 *
 * The best documentation I can find about these symbols is
 * https://arm-software.github.io/CMSIS_6/latest/Core/group__system__init__gr.html
 *
 * Currently, the implementation being used is from ST Micro.
 */

/*
 * SystemCoreClock is a global variable required by the ARM CMSIS code. It contains the core clock
 * value; note that the units are not specified, though examples tend to indicate that it's in Hz
 * (events per second or something similar).
 *
 * For more information, see
 * https://arm-software.github.io/CMSIS_5/Core/html/group__system__init__gr.html#gaa3cd3e43291e81e795d642b79b6088e6
 */
extern uint32_t SystemCoreClock;

/**
 * Function to update the core clock status variable.
 *
 * See
 * https://arm-software.github.io/CMSIS_5/Core/html/group__system__init__gr.html#gae0c36a9591fe6e9c45ecb21a794f0f0f
 * for a bit more information.
 */
void SystemCoreClockUpdate();
}

namespace tvsc::hal::boards::nucleo_h743zi {

/**
 * Class to manage the clock on the Nucleo H743ZI board.
 */
class ClockStm32H7xx final : public time::Clock {
 public:
  ClockStm32H7xx() {
    // For details on startup procedures, see stm32h7xx_hal_rcc.c. The comments in that file
    // explain many details that are otherwise difficult to find.

    // Also, the code in STM32Cube_FW_H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c can be a
    // useful reference.

    // We reinitialize SystemCoreClock as the startup process will have zeroed the BSS, likely
    // including SystemCoreClock. See the startup_<device>.s file for details on this process.
    SystemCoreClockUpdate();

    // Generate a tick interrupt every millisecond.
    SysTick_Config(SystemCoreClock / 1000);
  }

  // TODO(james): Add methods to support the proper clock interface, including some variations to
  // translate ticks into microseconds and milliseconds, detect and manage tick overflow, support
  // registering callback(s) either periodically or after a predetermined amount of time, etc. Note
  // that this interface should act as the HAL and not necessarily provide these functionalities
  // directly.

  time::TimeType current_time_millis() override { return 0; }
  time::TimeType current_time_micros() override { return 0; }

  void sleep_ms(time::TimeType milliseconds) override {}
  void sleep_us(time::TimeType microseconds) override {}
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
