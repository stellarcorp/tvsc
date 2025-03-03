#include "hal/board/simulation_board.h"

namespace tvsc::hal::board {

Board& Board::board() { return board_; }

Board::Board() {}

}  // namespace tvsc::hal::board

extern "C" {

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
void DMA1_Channel1_IRQHandler() {}

void LPTIM1_IRQHandler() {}

}  // extern "C"
