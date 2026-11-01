#pragma once

#include <array>
#include <chrono>
#include <cstddef>

#include "hal2/clock/clock_config.h"
#include "hal2/device.h"
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
struct DeviceDefinition;

template <>
struct DeviceDefinition<Facility::RCC_FACILITY, 0> final
    : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "core"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::RCC_FACILITY, 0};
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = rcc::Stm32L4RccAccessor<System>;

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array<DeviceId, 0>{};
  }
};

/**
 * LSI oscillator.
 */
template <>
struct DeviceDefinition<Facility::OSCILLATOR_FACILITY, 1> final
    : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "lsi"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::OSCILLATOR_FACILITY, 1};
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = rcc::Stm32L4LsiOscillatorAccessor<System>;
};

/**
 * Internal 48MHz oscillator used in USB commnications. For some reason, this oscillator is also
 * required by the RNG module.
 */
template <>
struct DeviceDefinition<Facility::OSCILLATOR_FACILITY, 2> final
    : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "hsi48"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::OSCILLATOR_FACILITY, 2};
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = rcc::Stm32L4Hsi48OscillatorAccessor<System>;
};

template <>
struct DeviceDefinition<Facility::SYSTICK_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "systick"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::SYSTICK_FACILITY, 0};
  }

  struct Configuration final {};
  using configuration = Configuration;

  static constexpr bool is_valid(const Configuration& config) noexcept { return true; }

  static constexpr auto dependencies(const Configuration& config) noexcept {
    return std::array{DeviceId{Facility::RCC_FACILITY, 0}};
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = systick::Stm32L4SysTickAccessor<System>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const Configuration& config) noexcept {
    table.add_arm_isr(SysTick_IRQn, systick::handle_systick_interrupt);
  }
};

template <size_t PORT>
  requires(PORT < 8)
struct DeviceDefinition<Facility::GPIO_FACILITY, PORT> final
    : public config::UnconfigurableDevice {
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

  static constexpr DeviceId id() noexcept {
    return {Facility::GPIO_FACILITY, PORT};
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = gpio::Stm32GpioAccessor<static_cast<gpio::PortNumber>(PORT), System>;
};

template <>
struct DeviceDefinition<Facility::LED_FACILITY, 0> final
    : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "debug_led"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::LED_FACILITY, 0};
  }

  static constexpr auto PIN{gpio::PinRef{2, 13}};

  static constexpr auto dependencies(const configuration& config) noexcept {
    return std::array{DeviceId{Facility::GPIO_FACILITY, PIN.port}};
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = led::LedAccessor<System, DeviceRegistry, 0, PIN>;
};

/**
 * Configuration for the low-power timers.
 */
template <size_t INDEX>
  requires(INDEX < 2)  // Restrict to low-power timers only.
struct DeviceDefinition<Facility::TIMER_FACILITY, INDEX> final
    : public config::UnconfigurableDevice {
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

  static constexpr DeviceId id() noexcept {
    return {Facility::TIMER_FACILITY, INDEX};
  }

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},        //
        DeviceId{Facility::OSCILLATOR_FACILITY, 1}  //
    };
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = timer::Stm32L4LowPowerTimerAccessor<System, DeviceRegistry, INDEX>;

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
struct DeviceDefinition<Facility::WATCHDOG_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "iwdg"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::WATCHDOG_FACILITY, 0};
  }

  struct Configuration final {
    uint32_t reset_interval_ms{2'000};
  };
  using configuration = Configuration;

  static constexpr bool is_valid(const Configuration& config) noexcept { return true; }

  static constexpr auto dependencies(const Configuration& config) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},        //
        DeviceId{Facility::OSCILLATOR_FACILITY, 1}  //
    };
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = watchdog::Stm32L4WatchdogAccessor<System, DeviceRegistry, config>;

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
struct DeviceDefinition<Facility::RNG_FACILITY, 0> final
    : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "rng"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::RNG_FACILITY, 0};
  }

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},        //
        DeviceId{Facility::OSCILLATOR_FACILITY, 2}  //
    };
  }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = random::Stm32RngAccessor<System, DeviceRegistry>;
};

/**
 * Configuration for the primary system clock.
 */
template <>
struct DeviceDefinition<Facility::CLOCK_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "steady_clock"; }

  static constexpr DeviceId id() noexcept {
    return {Facility::CLOCK_FACILITY, 0};
  }

  using configuration = clock::ClockConfiguration;

  static constexpr bool is_valid(const configuration&) noexcept { return true; }

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},      //
        DeviceId{Facility::SYSTICK_FACILITY, 0},  //
        DeviceId{Facility::TIMER_FACILITY, 1}     //
    };
  }

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = clock::ClockAccessor<System, DeviceRegistry, 0, CONFIG>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const configuration& config) noexcept {
    // No ISRs required.
  }
};

}  // namespace internal

struct Board final {
  template <Facility FACILITY, size_t INDEX>
  using definition = internal::DeviceDefinition<FACILITY, INDEX>;
};

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::RCC_FACILITY, 0>, Board>);
static_assert(
    config::ValidDeviceDefinition<
        typename Board::template definition<Facility::OSCILLATOR_FACILITY, 1>, Board>);

static_assert(
    config::ValidDeviceDefinition<
        typename Board::template definition<Facility::OSCILLATOR_FACILITY, 2>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::SYSTICK_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::GPIO_FACILITY, 0>, Board>);
static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::GPIO_FACILITY, 7>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::WATCHDOG_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::RNG_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::TIMER_FACILITY, 0>, Board>);
static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::TIMER_FACILITY, 1>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::LED_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template definition<Facility::CLOCK_FACILITY, 0>, Board>);

}  // namespace tvsc::hal2::board
