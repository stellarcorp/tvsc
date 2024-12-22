#pragma once

#include <array>
#include <cstdint>

#include "hal/boards/boards.h"
#include "hal/gpio/gpio.h"

extern "C" {

/**
 * These symbols are required by the ARM CMSIS code. They are typically defined in a file named
 * system_device.c. We declare them here so that they can be used in our code as well; if we're
 * paying for them, we're using them.
 *
 * It's not precisely clear exactly what is required of these symbols or why they are left to user
 * code to implement and manage. I suspect they were originally intended to be hooks to allow for
 * configurability, like the main() function, but instead, they became requirements of the
 * interface. With that mindset, these functions should be assumed to be the most minimal
 * implementation that makes the CPU work, and all other functionality should be available via other
 * abstractions.
 *
 * The best documentation I can find about these symbols is
 * https://arm-software.github.io/CMSIS_5/Core/html/group__system__init__gr.html
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

/**
 * Function required by ARM CMSIS structure to initialize the system.
 *
 * See
 * https://arm-software.github.io/CMSIS_5/Core/html/group__system__init__gr.html#ga93f514700ccf00d08dbdcff7f1224eb2
 * for a bit more information.
 */
void SystemInit();

/**
 * These symbols are required by the ST Micro implementation of the ARM core(s). Note that we do not
 * use these; our goal will be to remove these symbols and get the memory back. Doing so will also
 * validate that we have more command of the CPU and board and may be ready for more advanced
 * functionality.
 */

/**
 * This symbol is basically undocumented, but it appears to be the clock frequency of the second
 * core. As our system only has one core, SystemCoreClockUpdate() keeps its value equal to
 * SystemCoreClock.
 *
 * TODO(james): Remove this symbol as it is unnecessary in a single core system.
 */
extern uint32_t SystemD2CoreClock;

/**
 * Table of clock prescaler values. Not sure how useful this functionality is for us. At a minimum,
 * we should scope it to a class or function and not leave it as a global variable.
 *
 * TODO(james): Remove this symbol and move its functionality into a better abstraction.
 */
extern const uint8_t D1CorePrescTable[16];
}

namespace tvsc::hal::boards {

// Priority groups. More might be possible, but it's unclear if they would have any purpose.
// For more information, see NVIC_SetPriorityGrouping() in the ARM CMSIS code or documentation.
enum class NvicPriorityGroup : uint32_t {
  GROUP_0 = 7,
  GROUP_1 = 6,
  GROUP_2 = 5,
  GROUP_3 = 4,
  GROUP_4 = 3,
};

template <>
class Board<Boards::NUCLEO_H743ZI> final {
 public:
  static constexpr size_t NUM_GPIO_PORTS{4};
  static constexpr NvicPriorityGroup DEFAULT_PRIORITY_GROUP{NvicPriorityGroup::GROUP_4};
  static constexpr uint32_t TICK_INTERRUPT_PREEMPT_PRIORITY{0x0f};

 private:
  static constexpr std::array<uint32_t, 16> CLOCK_PRESCALING[];

  // Move these into the Clock implementation.
  uint32_t ticks_per_sec_;
  uint32_t tick_priority_encoded_;

  // TODO(james): Add details (memory offsets, most likely) to correctly initialize these GPIO
  // ports.
  gpio::Gpio gpio_port_a{};
  gpio::Gpio gpio_port_b{};
  gpio::Gpio gpio_port_c{};
  gpio::Gpio gpio_port_d{};

 public:
  Board();

  ~Board();

  template <size_t GPIO_PORT>
  gpio::Gpio& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    if constexpr (GPIO_PORT == 0) {
      return gpio_port_a;
    }
    if constexpr (GPIO_PORT == 1) {
      return gpio_port_b;
    }
    if constexpr (GPIO_PORT == 2) {
      return gpio_port_c;
    }
    if constexpr (GPIO_PORT == 3) {
      return gpio_port_d;
    }
  }
};

template <>
Board<Boards::NUCLEO_H743ZI>::Board() {
  // For details on startup procedures, see stm32h7xx_hal_rcc.c. The comments in that file
  // explain many details that are otherwise difficult to find.

  // The code in this method is loosely based on the HAL_Init() as found in
  // STM32Cube_FW_H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c

  uint32_t common_system_clock;

  /* Set Interrupt Group Priority */
  NVIC_SetPriorityGrouping(DEFAULT_PRIORITY_GROUP);

/* Update the system clock frequency */
#if defined(RCC_D1CFGR_D1CPRE)
  ticks_per_sec_ =
      HAL_RCC_GetSysClockFreq() >>
      ((D1CorePrescTable[(RCC->D1CFGR & RCC_D1CFGR_D1CPRE) >> RCC_D1CFGR_D1CPRE_Pos]) & 0x1FU);
#else
  ticks_per_sec_ =
      HAL_RCC_GetSysClockFreq() >>
      ((D1CorePrescTable[(RCC->CDCFGR1 & RCC_CDCFGR1_CDCPRE) >> RCC_CDCFGR1_CDCPRE_Pos]) & 0x1FU);
#endif

  /* Use systick as time base source and configure 1ms tick (default clock after Reset is HSI) */
  // if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK) {
  //   return HAL_ERROR;
  // }

  // Below here is loosely based on the HAL_InitTick() as found in
  // STM32Cube_FW_H7/Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c
  // It is an explicit function call in HAL_Init(), but the abundant use of global variables and
  // defines really defeats any concept of an abstraction boundary.

  /* Configure the SysTick interrupt rate. This is the number of ticks in between interrupt calls.
   * The default value for uwTickFreq is 1, giving an argument of ticks per second divided by 1000,
   * or ticks per millisecond. With the default value for uwTickFreq, this gives one interrupt call
   * per millisecond.
   */
  const uint32_t num_ticks_between_interrupts{ticks_per_sec_ / (1000UL / (uint32_t)uwTickFreq)};
  if (SysTick_Config(num_ticks_between_interrupts) > 0) {
    // TODO(james): Find a way to identify errors at this stage of the boot process.
  }

  /* Configure the SysTick IRQ priority */
  // Note that SysTick_Config() sets this interrupt to (what appears to be) the highest possible
  // priority ((1UL << __NVIC_PRIO_BITS) - 1UL). Not sure why.
  // TODO(james): Validate that we aren't setting this priority twice for no valid reason.
  tick_priority_encoded_ =
      NVIC_EncodePriority(DEFAULT_PRIORITY_GROUP, TICK_INTERRUPT_PREEMPT_PRIORITY, 0);
  NVIC_SetPriority(SysTick_IRQn, tick_priority_encoded_);

  // Needed for anything?
  // uwTickPrio = TICK_INT_PRIORITY;
}

}  // namespace tvsc::hal::boards
