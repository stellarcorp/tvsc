#pragma once

#include <array>

#include "hal2/config/facility.h"
#include "hal2/config/vector_table.h"
#include "hal2/gpio/gpio.h"
#include "hal2/gpio/stm32_gpio_config.h"
#include "hal2/led/led_config.h"
#include "hal2/rcc/stm32l4_rcc_config.h"
#include "hal2/systick/stm32l4_systick_config.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::board {

namespace internal {

template <config::Facility FACILITY, size_t INDEX>
struct PeripheralConfiguration;

template <>
struct PeripheralConfiguration<config::Facility::RCC_FACILITY, 0> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = rcc::Stm32L4RccAccessor<System>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::RCC_FACILITY, 0};
  }
  static constexpr auto dependencies() noexcept { return std::array<config::PeripheralId, 0>{}; }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

template <>
struct PeripheralConfiguration<config::Facility::OSCILLATOR_FACILITY, 1> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = rcc::Stm32L4LsiOscillatorAccessor<System>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::OSCILLATOR_FACILITY, 1};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{config::PeripheralId{config::Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

template <>
struct PeripheralConfiguration<config::Facility::OSCILLATOR_FACILITY, 2> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = rcc::Stm32L4Hsi48OscillatorAccessor<System>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::OSCILLATOR_FACILITY, 2};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{config::PeripheralId{config::Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

template <>
struct PeripheralConfiguration<config::Facility::SYSTICK_FACILITY, 0> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = systick::Stm32L4SysTickAccessor<System>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::SYSTICK_FACILITY, 0};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{config::PeripheralId{config::Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    table.add_arm_isr(SysTick_IRQn, systick::handle_systick_interrupt);
  }
};

template <size_t PORT>
struct PeripheralConfiguration<config::Facility::GPIO_FACILITY, PORT> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = gpio::Stm32GpioAccessor<static_cast<gpio::PortNumber>(PORT), System>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::GPIO_FACILITY, PORT};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{config::PeripheralId{config::Facility::RCC_FACILITY, 0}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

template <size_t INDEX>
  requires(INDEX < 1)
struct PeripheralConfiguration<config::Facility::LED_FACILITY, INDEX> final {
  static constexpr std::array PINS{
      gpio::PinRef{2, 13}  //
  };

  template <typename System, typename PeripheralRegistry>
  using accessor = led::LedAccessor<System, PeripheralRegistry, INDEX, PINS[INDEX]>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::LED_FACILITY, INDEX};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{config::PeripheralId{config::Facility::GPIO_FACILITY, PINS[INDEX].port}};
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

}  // namespace internal

struct Board final {
  template <config::Facility FACILITY, size_t INDEX>
  using configuration = internal::PeripheralConfiguration<FACILITY, INDEX>;
};

static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::RCC_FACILITY, 0>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::OSCILLATOR_FACILITY, 1>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::OSCILLATOR_FACILITY, 2>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::SYSTICK_FACILITY, 0>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::GPIO_FACILITY, 0>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::GPIO_FACILITY, 7>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::LED_FACILITY, 0>, Board>);

}  // namespace tvsc::hal2::board
