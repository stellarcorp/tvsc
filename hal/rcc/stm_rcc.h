#pragma once

#include <cstddef>
#include <cstdint>
#include <new>

#include "hal/gpio/gpio.h"
#include "hal/rcc/rcc.h"
#include "hal/register.h"
#include "hal/time/clock.h"

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

/**
 * Current time in microseconds where "current time" means time since last reset.
 */
extern volatile CTimeType current_time_us;

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
 * Interrupt handler for SysTick. Handles scenario when the SysTick counter
 * (SysTickRegisterBank::VAL) reaches zero and reloads its value from SysTickRegisterBank::LOAD.
 */
void SysTick_Handler();
}

namespace tvsc::hal::rcc {

class RccRegisterBank final {
 public:
  // Clock source register. This register determines which clocks are in use. It includes flags for
  // turning on the various clocks and PLLs, as well as flags to determine if those clock sources
  // are ready to be used.
  // Offset 0x000
  volatile Register CR;

  std::byte unused1[0x010 - sizeof(Register)];

  // Clock configuration register. This register has several functions. It controls the source of
  // the MCO clock outputs. It controls the prescalers for the timers. It controls which clock is
  // used on a wakeup from system stop. And it controls which clock source is used for the system
  // clock.
  // Offset 0x010
  volatile Register CFGR;

  std::byte unused2[0x060 - 0x10 - sizeof(Register)];

  // Clock source interrupt enable register. The bits in this register enable/disable the interrupt
  // for the ready signal of the various clock sources.
  // Offset 0x060
  volatile Register CIER;

  std::byte unused3[0x0e0 - 0x60 - sizeof(Register)];

  // Offset 0x0e0
  volatile Register AHB4ENR;

  std::byte unused4[0x0f4 - 0x0e0 - sizeof(Register)];

  // Offset 0x0f4
  volatile Register APB4ENR;
};

class SysTickRegisterBank final {
 public:
  //
  volatile Register CTRL;
  volatile Register LOAD;
  volatile Register VAL;
  volatile Register CALIB;
};

/**
 * Class to manage the reset and clock circuitry (RCC) on the Nucleo H743ZI board.
 */
class RccStm32H7xx final : public Rcc {
 private:
  RccRegisterBank* const rcc_registers_;
  SysTickRegisterBank* const sys_tick_registers_;

  void update_sys_tick();

 public:
  RccStm32H7xx(void* rcc_base_address, void* sys_tick_base_address)
      : rcc_registers_(new (rcc_base_address) RccRegisterBank),
        sys_tick_registers_(new (sys_tick_base_address) SysTickRegisterBank) {
    // For details on startup procedures, see stm32h7xx_hal_rcc.c. The comments in that file
    // explain many details that are otherwise difficult to find.

    // Also, the code in STM32Cube_FW_H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c can be a
    // useful reference.

    // We reinitialize SystemCoreClock as the startup process will have zeroed the BSS, likely
    // including SystemCoreClock. See the startup_<device>.s file for details on this process.
    SystemCoreClockUpdate();

    update_sys_tick();
  }

  void enable_gpio_port(gpio::Port port) override;
  void disable_gpio_port(gpio::Port port) override;

  void set_clock_to_max_speed() override;
  void set_clock_to_min_speed() override;
};

}  // namespace tvsc::hal::rcc
