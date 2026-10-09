#include "hal/mcu/stm32l4xx.h"

#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::mcu::internal {

void configure_interrupts() {
  // SysTick interrupt(s).
  HAL_NVIC_SetPriority(SysTick_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(SysTick_IRQn);

  // LPTIM1 interrupt(s).
  HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
}

}  // namespace tvsc::hal::mcu::internal

extern "C" {

/**
 * The ISRs below are declared in the startup assembly (startup_<device>.s for STM32 devices). They
 * are also given weak, default implementations to call a default handler. The default handler just
 * loops forever. Presumably, we would configure some form of watchdog to reboot in that event. The
 * implementations here are designed to allow for better bringup and debugging. Later, we probably
 * will want to cull this set of definitions to ones we actually use as each of these represent a
 * tiny amount of memory overhead.
 */
// TODO(james): Fix names of interrupt handlers. These are the default names from ST Micro, and they
// are inconsistent with the naming in the rest of the project.

void LPTIM1_IRQHandler() {
  tvsc::hal::mcu::Mcu& mcu{tvsc::hal::mcu::Mcu::mcu()};
  mcu.sleep_timer().handle_interrupt();
}

void SysTick_Handler() {
  tvsc::hal::mcu::Mcu& mcu{tvsc::hal::mcu::Mcu::mcu()};
  mcu.sys_tick().handle_interrupt();
}

}  // extern "C"
