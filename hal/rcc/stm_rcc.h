#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

#include "hal/gpio/gpio.h"
#include "hal/rcc/rcc.h"
#include "hal/register.h"

extern "C" {

/**
 * These symbols are usually required by the ARM CMSIS code. They are typically defined in a file
 * named system_<device>.c. We declare them here so that they can be used in our code as well; if
 * we're paying for them, we're using them.
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
 * value in ticks per second.
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

namespace tvsc::hal::rcc {

class RccRegisterBank final {
 public:
  std::byte unused1[0x0e0];
  // Offset 0x0e0
  volatile Register AHB4ENR;
  std::byte unused2[0x0f4 - 0x0e0];
  // Offset 0x0f4
  volatile Register APB4ENR;
};

/**
 * Class to manage the reset and clock circuitry (RCC) on the Nucleo H743ZI board.
 */
class RccStm32H7xx final : public Rcc {
 private:
  RccRegisterBank* registers_;

 public:
  RccStm32H7xx(void* base_address) : registers_(new (base_address) RccRegisterBank) {
    // For details on startup procedures, see stm32h7xx_hal_rcc.c. The comments in that file
    // explain many details that are otherwise difficult to find.

    // Also, the code in STM32Cube_FW_H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c can be a
    // useful reference.

    // We reinitialize SystemCoreClock as the startup process will have zeroed the BSS, likely
    // including SystemCoreClock. See the startup_<device>.s file for details on this process.
    SystemCoreClockUpdate();
  }

  void enable_gpio_port(gpio::Port port) override;
  void disable_gpio_port(gpio::Port port) override;
};

}  // namespace tvsc::hal::rcc
