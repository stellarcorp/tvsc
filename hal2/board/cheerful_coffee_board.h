#pragma once

#include <array>
#include <chrono>
#include <cstddef>

#include "hal2/clock/clock_config.h"
#include "hal2/peripheral.h"
#include "hal2/config/vector_table.h"
#include "hal2/gpio/gpio.h"
#include "hal2/gpio/stm32_gpio_config.h"
#include "hal2/led/led_config.h"
#include "hal2/random/stm32_rng_config.h"
#include "hal2/rcc/stm32l4_rcc_config.h"
#include "hal2/systick/stm32l4_systick_config.h"
#include "hal2/timer/stm32l4_timer_config.h"
#include "hal2/watchdog/stm32l4_watchdog_config.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::board {

namespace internal {

template <Facility FACILITY, size_t INDEX>
struct PeripheralDefinition;

template <>
struct PeripheralDefinition<Facility::RCC_FACILITY, 0> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept { return "core"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::RCC_FACILITY, 0};
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = rcc::Stm32L4RccAccessor<System>;

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array<PeripheralId, 0>{};
  }
};

/**
 * LSI oscillator.
 */
template <>
struct PeripheralDefinition<Facility::OSCILLATOR_FACILITY, 1> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept { return "lsi"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::OSCILLATOR_FACILITY, 1};
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = rcc::Stm32L4LsiOscillatorAccessor<System>;
};

/**
 * Internal 48MHz oscillator used in USB commnications. For some reason, this oscillator is also
 * required by the RNG module.
 */
template <>
struct PeripheralDefinition<Facility::OSCILLATOR_FACILITY, 2> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept { return "hsi48"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::OSCILLATOR_FACILITY, 2};
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = rcc::Stm32L4Hsi48OscillatorAccessor<System>;
};

template <>
struct PeripheralDefinition<Facility::SYSTICK_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "systick"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::SYSTICK_FACILITY, 0};
  }

  struct Configuration final {};
  using configuration = Configuration;

  static constexpr bool is_valid(const Configuration& config) noexcept { return true; }

  static constexpr auto dependencies(const Configuration& config) noexcept {
    return std::array{PeripheralId{Facility::RCC_FACILITY, 0}};
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = systick::Stm32L4SysTickAccessor<System>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const Configuration& config) noexcept {
    table.add_arm_isr(SysTick_IRQn, systick::handle_systick_interrupt);
  }
};

template <size_t PORT>
  requires(PORT < 8)
struct PeripheralDefinition<Facility::GPIO_FACILITY, PORT> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept
    requires(PORT == 0)
  {
    return "gpioa";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 1)
  {
    return "gpiob";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 2)
  {
    return "gpioc";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 3)
  {
    return "gpiod";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 4)
  {
    return "gpioe";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 5)
  {
    return "gpiof";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 6)
  {
    return "gpiog";
  }
  static constexpr const char* name() noexcept
    requires(PORT == 7)
  {
    return "gpioh";
  }

  static constexpr PeripheralId id() noexcept {
    return {Facility::GPIO_FACILITY, PORT};
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = gpio::Stm32GpioAccessor<static_cast<gpio::PortNumber>(PORT), System>;
};

template <>
struct PeripheralDefinition<Facility::LED_FACILITY, 0> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept { return "debug_led"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::LED_FACILITY, 0};
  }

  static constexpr auto PIN{gpio::PinRef{2, 13}};

  static constexpr auto dependencies(const configuration& config) noexcept {
    return std::array{PeripheralId{Facility::GPIO_FACILITY, PIN.port}};
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = led::LedAccessor<System, PeripheralRegistry, 0, PIN>;
};

/**
 * Configuration for the low-power timers.
 */
template <size_t INDEX>
  requires(INDEX < 2)  // Restrict to low-power timers only.
struct PeripheralDefinition<Facility::TIMER_FACILITY, INDEX> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept
    requires(INDEX == 0)
  {
    return "lptim1";
  }
  static constexpr const char* name() noexcept
    requires(INDEX == 1)
  {
    return "lptim2";
  }

  static constexpr PeripheralId id() noexcept {
    return {Facility::TIMER_FACILITY, INDEX};
  }

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        PeripheralId{Facility::RCC_FACILITY, 0},        //
        PeripheralId{Facility::OSCILLATOR_FACILITY, 1}  //
    };
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = timer::Stm32L4LowPowerTimerAccessor<System, PeripheralRegistry, INDEX>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const configuration&) noexcept {
    if constexpr (INDEX == 0) {
      table.add_arm_isr(LPTIM1_IRQn, timer::handle_low_power_timer1_interrupt);
    } else if constexpr (INDEX == 1) {
      table.add_arm_isr(LPTIM2_IRQn, timer::handle_low_power_timer2_interrupt);
    }
  }
};

/**
 * Configuration for the watchdog
 */
template <>
struct PeripheralDefinition<Facility::WATCHDOG_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "iwdg"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::WATCHDOG_FACILITY, 0};
  }

  struct Configuration final {
    uint32_t reset_interval_ms{2'000};
  };
  using configuration = Configuration;

  static constexpr bool is_valid(const Configuration& config) noexcept { return true; }

  static constexpr auto dependencies(const Configuration& config) noexcept {
    return std::array{
        PeripheralId{Facility::RCC_FACILITY, 0},        //
        PeripheralId{Facility::OSCILLATOR_FACILITY, 1}  //
    };
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = watchdog::Stm32L4WatchdogAccessor<System, PeripheralRegistry, config>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const Configuration& config) noexcept {
    // No ISRs required.
  }
};

/**
 * Configuration for the RNG
 */
template <>
struct PeripheralDefinition<Facility::RNG_FACILITY, 0> final
    : public config::UnconfigurablePeripheral {
  static constexpr const char* name() noexcept { return "rng"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::RNG_FACILITY, 0};
  }

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        PeripheralId{Facility::RCC_FACILITY, 0},        //
        PeripheralId{Facility::OSCILLATOR_FACILITY, 2}  //
    };
  }

  template <typename System, typename PeripheralRegistry, configuration config>
  using accessor = random::Stm32RngAccessor<System, PeripheralRegistry>;
};

/**
 * Configuration for the primary system clock.
 */
template <>
struct PeripheralDefinition<Facility::CLOCK_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "steady_clock"; }

  static constexpr PeripheralId id() noexcept {
    return {Facility::CLOCK_FACILITY, 0};
  }

  using configuration = clock::ClockConfiguration;

  static constexpr bool is_valid(const configuration&) noexcept { return true; }

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        PeripheralId{Facility::RCC_FACILITY, 0},      //
        PeripheralId{Facility::SYSTICK_FACILITY, 0},  //
        PeripheralId{Facility::TIMER_FACILITY, 1}     //
    };
  }

  template <typename System, typename PeripheralRegistry, configuration CONFIG>
  using accessor = clock::ClockAccessor<System, PeripheralRegistry, 0, CONFIG>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const configuration& config) noexcept {
    // No ISRs required.
  }
};

}  // namespace internal

struct Board final {
  template <Facility FACILITY, size_t INDEX>
  using definition = internal::PeripheralDefinition<FACILITY, INDEX>;
};

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::RCC_FACILITY, 0>, Board>);
static_assert(
    config::ValidPeripheralDefinition<
        typename Board::template definition<Facility::OSCILLATOR_FACILITY, 1>, Board>);

static_assert(
    config::ValidPeripheralDefinition<
        typename Board::template definition<Facility::OSCILLATOR_FACILITY, 2>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::SYSTICK_FACILITY, 0>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::GPIO_FACILITY, 0>, Board>);
static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::GPIO_FACILITY, 7>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::WATCHDOG_FACILITY, 0>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::RNG_FACILITY, 0>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::TIMER_FACILITY, 0>, Board>);
static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::TIMER_FACILITY, 1>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::LED_FACILITY, 0>, Board>);

static_assert(config::ValidPeripheralDefinition<
              typename Board::template definition<Facility::CLOCK_FACILITY, 0>, Board>);

}  // namespace tvsc::hal2::board
