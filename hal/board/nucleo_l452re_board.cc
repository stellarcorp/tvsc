#include "hal/board/nucleo_l452re_board.h"

#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal::board {

Board& Board::board() { return board_; }

Board::Board() {
  // DMA interrupt(s).
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  // LPTIM1 interrupt(s).
  HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
}

}  // namespace tvsc::hal::board

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

/**
 * Non-maskable interrupt handler.
 */
void NMI_Handler(void) {
  while (1) {
  }
}

/**
 * Hard fault interrupt handler.
 */
void HardFault_Handler(void) {
  while (1) {
  }
}

/**
 * Memory fault handler.
 */
void MemManage_Handler(void) {
  while (1) {
  }
}

/**
 * Handler for memory access faults, including prefetch errors.
 */
void BusFault_Handler(void) {
  while (1) {
  }
}

/**
 * Undefined instruction handler.
 */
void UsageFault_Handler(void) {
  while (1) {
  }
}

/**
 * System service call (SWI instruction) handler.
 */
void SVC_Handler(void) {}

/**
 * Debug monitor.
 */
void DebugMon_Handler(void) {}

/**
 * Pending system service call handler.
 */
void PendSV_Handler(void) {}

/**
 * DMA interrupt handler. Note that the specific reason this interrupt is triggered will vary from
 * board to board.
 */
void DMA1_Channel1_IRQHandler() {
  tvsc::hal::board::Board& board{tvsc::hal::board::Board::board()};
  board.dma().handle_interrupt();
}

void LPTIM1_IRQHandler() {
  tvsc::hal::board::Board& board{tvsc::hal::board::Board::board()};
  board.sleep_timer().handle_interrupt();
}

void SysTick_Handler() {
  tvsc::hal::board::Board& board{tvsc::hal::board::Board::board()};
  board.sys_tick().handle_interrupt();
}

}  // extern "C"
