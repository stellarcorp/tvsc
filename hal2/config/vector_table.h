#pragma once

#include <array>
#include <limits>
#include <stdexcept>

#include "base/except.h"

extern "C" char _estack[];
extern "C" void Reset_Handler(void);

namespace tvsc::hal2::config {

[[noreturn]] inline void DefaultInterruptHandler() {
  while (true) {
    // Loop indefinitely. Do not return.
  }
}

template <size_t NUM_ISRS>
class VectorTable final {
 public:
  using Isr = void (*)();

 private:
  void* stack_end{_estack};
  std::array<Isr, NUM_ISRS> isrs{};

  static constexpr int ARM_IRQn_OFFSET{15};

  constexpr size_t validate_index(int index) const {
    if (index < -14) [[unlikely]] {
      except<std::out_of_range>(
          "ISR index below valid ARM core exception range. ARM IRQn's start with "
          "NonMaskableInt_IRQn, which has a value of -14.");
    }

    if (index > std::numeric_limits<int>::max() - ARM_IRQn_OFFSET) [[unlikely]] {
      except<std::out_of_range>("Outrageously large ISR index.");
    }

    size_t actual_index{static_cast<size_t>(index + ARM_IRQn_OFFSET)};

    if (actual_index >= isrs.size()) [[unlikely]] {
      except<std::out_of_range>(
          "ISR index out of bounds in interrupt vector table configuration. Likely, the vector "
          "table (NUM_ISRS) is too small and is missing space for Reset_Handler. The interrupt "
          "vector table needs space for Reset_Handler and all configured ISRs, including ARM's "
          "core interrupts (NonMaskableInt_IRQn to SysTick_IRQn). The VectorTable class already "
          "allocates space separately for the end of the stack.");
    }

    return actual_index;
  }

 public:
  constexpr VectorTable() noexcept : VectorTable(_estack, &Reset_Handler) {}

  constexpr explicit VectorTable(void* custom_stack_end) noexcept
      : VectorTable(custom_stack_end, &Reset_Handler) {}

  constexpr VectorTable(void* custom_stack_end, Isr custom_reset_handler) noexcept
      : stack_end(custom_stack_end) {
    isrs[0] = custom_reset_handler;
    for (size_t i = 1; i < isrs.size(); ++i) {
      isrs[i] = &DefaultInterruptHandler;
    }
  }

  /**
   * Add an ISR at the index as provided by ARM and manufactorers using ARM cores. These indices
   * start at -14 for the NonMaskableInt_IRQn and set up most projects for off-by-one bugs on the
   * interrupt vector table configuration.
   */
  constexpr void add_arm_isr(int index, Isr isr) noexcept {
    size_t actual_index{validate_index(index)};

    // Guard against collisions.
    if (isrs[actual_index] == &DefaultInterruptHandler) {
      isrs[actual_index] = isr;
    } else [[unlikely]] {
      except<std::logic_error>("Collision in interrupt vector table configuration.");
    }
  }

  /**
   * Read back the value of the function pointer in the interrupt vector table.
   */
  [[nodiscard]] constexpr Isr at_arm_isr(int index) const noexcept {
    return isrs[validate_index(index)];
  }

  constexpr VectorTable(const VectorTable&) noexcept = default;
  constexpr VectorTable(VectorTable&&) noexcept = default;
  constexpr VectorTable& operator=(const VectorTable&) noexcept = default;
  constexpr VectorTable& operator=(VectorTable&&) noexcept = default;
};

}  // namespace tvsc::hal2::config
