#pragma once

#include <array>

#include "hal2/config/facility.h"
#include "hal2/config/vector_table.h"
#include "hal2/gpio/gpio.h"
#include "hal2/gpio/stm32_gpio_config.h"
#include "hal2/led/led_config.h"
#include "hal2/rcc/stm32l4_rcc_config.h"
#include "hal2/systick/stm32l4_systick_config.h"
#include "hal2/time/clock_config.h"
#include "hal2/timer/stm32l4_timer_config.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::board::internal {

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

/**
 * LSI oscillator.
 */
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

/**
 * Internal 48MHz oscillator used in USB commnications. For some reason, this oscillator is also
 * required by the RNG module.
 */
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

/**
 * Configuration for the low-power timers.
 */
template <size_t INDEX>
  requires(INDEX < 2)  // Restrict to low-power timers only.
struct PeripheralConfiguration<config::Facility::TIMER_FACILITY, INDEX> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = timer::Stm32L4LowPowerTimerAccessor<System, PeripheralRegistry, INDEX>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::TIMER_FACILITY, INDEX};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{
        config::PeripheralId{config::Facility::RCC_FACILITY, 0},        //
        config::PeripheralId{config::Facility::OSCILLATOR_FACILITY, 1}  //
    };
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    if constexpr (INDEX == 0) {
      table.add_arm_isr(LPTIM1_IRQn, timer::handle_low_power_timer1_interrupt);
    } else if constexpr (INDEX == 1) {
      table.add_arm_isr(LPTIM2_IRQn, timer::handle_low_power_timer2_interrupt);
    }
  }
};

/**
 * Configuration for the primary system clock.
 */
template <size_t INDEX>
  requires(INDEX == 0)  // Restrict to low-power timers only.
struct PeripheralConfiguration<config::Facility::CLOCK_FACILITY, INDEX> final {
  template <typename System, typename PeripheralRegistry>
  using accessor = time::ClockAccessor<System, PeripheralRegistry, INDEX>;

  static constexpr config::PeripheralId id() noexcept {
    return {config::Facility::CLOCK_FACILITY, INDEX};
  }
  static constexpr auto dependencies() noexcept {
    return std::array{
        config::PeripheralId{config::Facility::RCC_FACILITY, 0},      //
        config::PeripheralId{config::Facility::SYSTICK_FACILITY, 0},  //
        config::PeripheralId{config::Facility::TIMER_FACILITY, 1}     //
    };
  }

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table) noexcept {
    // No ISRs required.
  }
};

}  // namespace tvsc::hal2::board::internal
