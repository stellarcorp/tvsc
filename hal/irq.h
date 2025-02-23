#pragma once

namespace tvsc::hal {

/**
 * Disable interrupts.
 */
__attribute__((always_inline)) static inline void disable_irq(void) {
  __asm volatile("cpsid i" : : : "memory");
}

/**
 * Enable interrupts.
 */
__attribute__((always_inline)) static inline void enable_irq(void) {
  __asm volatile("cpsie i" : : : "memory");
}

}  // namespace tvsc::hal
