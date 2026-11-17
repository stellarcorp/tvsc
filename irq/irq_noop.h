#pragma once

namespace tvsc::irq {

__attribute__((always_inline)) static inline void disable_irq(void) {
  // Do nothing.
}

__attribute__((always_inline)) static inline void enable_irq(void) {
  // Do nothing.
}

}  // namespace tvsc::irq
