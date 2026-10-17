#pragma once

#include <array>

extern "C" char _estack[];
extern "C" void Reset_Handler(void);

namespace tvsc::hal2::config {

[[noreturn]] inline void DefaultInterruptHandler() {
  while (true) {
    // Loop indefinitely. Do not return.
  }
}

template <size_t NUM_ISRS>
struct VectorTable final {
  using Isr = void (*)();

  void* stack_end{_estack};
  std::array<Isr, NUM_ISRS> isrs{};

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

  constexpr VectorTable(const VectorTable&) noexcept = default;
  constexpr VectorTable(VectorTable&&) noexcept = default;
  constexpr VectorTable& operator=(const VectorTable&) noexcept = default;
  constexpr VectorTable& operator=(VectorTable&&) noexcept = default;
};

}  // namespace tvsc::hal2::config
