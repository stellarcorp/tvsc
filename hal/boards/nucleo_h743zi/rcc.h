#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <new>

#include "hal/boards/register.h"
#include "hal/gpio/pins.h"

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

/**
 * Function to handle early system initialization, including clock selection. It is called from the
 * ResetHandler in the startup_<device>.s file. This routine was provided by ST Micro.
 *
 * TODO(james): Modify this routine to fit our use case. In particular, switch to the CSI oscillator
 * by default, rather than using the HSE or HSI oscillators.
 */
void SystemInit(void);
}

namespace tvsc::hal::boards::nucleo_h743zi {

class RccRegisterBank final {
 public:
  std::byte unused1[0x0e0];
  // Offset 0x0e0
  volatile Register AHB4ENR;
};

/**
 * Class to manage the reset and clock circuitry (RCC) on the Nucleo H743ZI board.
 */
class Rcc final {
 private:
  RccRegisterBank* registers_;

 public:
  Rcc(void* base_address) : registers_(new (base_address) RccRegisterBank) {
    // For details on startup procedures, see stm32h7xx_hal_rcc.c. The comments in that file
    // explain many details that are otherwise difficult to find.

    // Also, the code in STM32Cube_FW_H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c can be a
    // useful reference.

    // We reinitialize SystemCoreClock as the startup process will have zeroed the BSS, likely
    // including SystemCoreClock. See the startup_<device>.s file for details on this process.
    SystemCoreClockUpdate();
  }

  template <gpio::Port GPIO_PORT>
  void enable_port() {
    registers_->AHB4ENR.set_bit_field_value_and_block<1>(1, static_cast<uint8_t>(GPIO_PORT));
  }

  template <gpio::Port GPIO_PORT>
  void disable_port() {
    registers_->AHB4ENR.set_bit_field_value_and_block<1>(0, static_cast<uint8_t>(GPIO_PORT));
  }
};

}  // namespace tvsc::hal::boards::nucleo_h743zi
