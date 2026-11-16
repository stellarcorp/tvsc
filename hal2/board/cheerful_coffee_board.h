#pragma once

#include <array>
#include <chrono>
#include <cstddef>

#include "hal2/adc/stm32l4_adc_config.h"
#include "hal2/board_identification/board_identification_config.h"
#include "hal2/clock/clock_config.h"
#include "hal2/config/device_definitions.h"
#include "hal2/config/vector_table.h"
#include "hal2/device.h"
#include "hal2/dma/stm32l4_dma_config.h"
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

/**
 * Structure defining the RCC (Reset and Clock Configuration) of the MCU. This device manages the
 * core clock speeds and voltage regulator for the MCU.
 *
 * Note that this struct is also commented to describe how DeviceDefinitions should be set up. Many
 * of the comments below pertain to how to use a DeviceDefinition, rather than the definition of the
 * RCC device itself. See the LSI oscillator device definition just below the RCC for an example of
 * a more minimal device definition.
 *
 * A device definition describes a single device -- peripheral, bus, or some other facility -- on a
 * board or on the MCU. These device definitions are brought together into a Board type that acts as
 * a registry of all of the devices on the board. The devices are defined from the perspective of
 * the MCU running this code. On a board with multiple MCUs, you might require separate sets of
 * device definitions, one set for each MCU.
 */
template <>
struct DeviceDefinition<Facility::RCC_FACILITY, 0> final {
  // All device definitions provide a name that is a C-string literal. The name should be consistent
  // with the name given in the datasheet. For STM32 parts, this usually means that the name is
  // one-based, even if all other indexiing in our codebase is zero-based. Making the names
  // consistent with the datasheet sometimes helps and sometimes means understanding another set of
  // conventions.
  static constexpr const char* name() noexcept { return "rcc"; }

  // All device definitions also provide the ID of the device they define. The device id is just the
  // tuple of the facility and an index. Ideally, the indices within a facility are monotonically
  // increasing and start at zero, though there is no technical reason for this limitation.
  static constexpr DeviceId id() noexcept { return {Facility::RCC_FACILITY, 0}; }

  // Device definitions can provide a configuration type. These configuration types can be used to
  // configure the device when building the system. In the case of RCC, the configuration includes a
  // clock speed. This allows the clock speed of the MCU to be configured at compile-time.
  using configuration = rcc::RccConfiguration;

  // Device definitions should provide this method to validate their configuration. Here, the
  // configuration is constrained with enums so that it can't be invalid.
  static constexpr bool is_valid(const configuration&) noexcept { return true; }

  // How to access the instance of this device. The System instance will inherit from all of these
  // accessors so as to provide a runtime mechanism for accessing each device. The SystemBuilder is
  // used to determine which accessors must be included in the System; that determination is done at
  // compile-time.
  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = rcc::Stm32L4RccAccessor<CONFIG>;

  // Each device definition can provide a list of dependencies. Any dependencies are guaranteed to
  // be brought into the System without explicitly requiring mention in the SystemBuilder.
  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array<DeviceId, 0>{};
  }

  // Finally, each device can add any number of ISRs to the interrupt vector table. The vector table
  // is built at compile-time and can be placed in flash memory.
  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>&,
                                            const configuration&) noexcept {
    // No ISRs required by device.
  }
};

/**
 * LSI oscillator.
 *
 * The definition of the LSI is considerably simpler than the RCC above. The LSI has no
 * configuration parameters, so it can inherit from a type which handles the boilerplate for devices
 * that can't be configured.
 */
template <>
struct DeviceDefinition<Facility::OSCILLATOR_FACILITY, 1> final
    : public config::UnconfigurableDevice {
  // Unconfigurable devices still need to provide a name as a C-string literal.
  static constexpr const char* name() noexcept { return "lsi"; }

  // Unconfigurable devices need to provide an id.
  static constexpr DeviceId id() noexcept { return {Facility::OSCILLATOR_FACILITY, 1}; }

  // And the accessor must be provided in order to construct the System type.
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

  static constexpr DeviceId id() noexcept { return {Facility::OSCILLATOR_FACILITY, 2}; }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = rcc::Stm32L4Hsi48OscillatorAccessor<System>;
};

template <>
struct DeviceDefinition<Facility::SYSTICK_FACILITY, 0> final {
  static constexpr const char* name() noexcept { return "systick"; }

  static constexpr DeviceId id() noexcept { return {Facility::SYSTICK_FACILITY, 0}; }

  // TODO(james): Either remove this struct and inherit from UnconfigurableDevice, or make it useful
  // (possibly allow the configuration of the tick frequency).
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
struct DeviceDefinition<Facility::GPIO_FACILITY, PORT> final : public config::UnconfigurableDevice {
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

  static constexpr DeviceId id() noexcept { return {Facility::GPIO_FACILITY, PORT}; }

  template <typename System, typename DeviceRegistry, configuration config>
  using accessor = gpio::Stm32GpioAccessor<static_cast<gpio::PortNumber>(PORT), System>;
};

template <>
struct DeviceDefinition<Facility::LED_FACILITY, 0> final : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "debug_led"; }

  static constexpr DeviceId id() noexcept { return {Facility::LED_FACILITY, 0}; }

  static constexpr auto PIN{gpio::PinRef{/* GPIO Bank C */ 2, 13}};

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

  static constexpr DeviceId id() noexcept { return {Facility::TIMER_FACILITY, INDEX}; }

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

  static constexpr DeviceId id() noexcept { return {Facility::WATCHDOG_FACILITY, 0}; }

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
struct DeviceDefinition<Facility::RNG_FACILITY, 0> final : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "rng"; }

  static constexpr DeviceId id() noexcept { return {Facility::RNG_FACILITY, 0}; }

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

  static constexpr DeviceId id() noexcept { return {Facility::CLOCK_FACILITY, 0}; }

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

/**
 * Configuration for the two DMA controllers.
 */
template <>
struct DeviceDefinition<Facility::DMA_FACILITY, 0> final : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "dma1"; }

  static constexpr DeviceId id() noexcept { return {Facility::DMA_FACILITY, 0}; }

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = dma::Stm32L4DmaAccessor<0>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const configuration& config) noexcept {
    // TODO(james): Move these to the device definitions for the DMA channels.
    table.add_arm_isr(DMA1_Channel1_IRQn, dma::handle_interrupt<0, 0>);
    table.add_arm_isr(DMA1_Channel2_IRQn, dma::handle_interrupt<0, 1>);
    table.add_arm_isr(DMA1_Channel3_IRQn, dma::handle_interrupt<0, 2>);
    table.add_arm_isr(DMA1_Channel4_IRQn, dma::handle_interrupt<0, 3>);
    table.add_arm_isr(DMA1_Channel5_IRQn, dma::handle_interrupt<0, 4>);
    table.add_arm_isr(DMA1_Channel6_IRQn, dma::handle_interrupt<0, 5>);
    table.add_arm_isr(DMA1_Channel7_IRQn, dma::handle_interrupt<0, 6>);
  }
};

template <>
struct DeviceDefinition<Facility::DMA_FACILITY, 1> final : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "dma2"; }

  static constexpr DeviceId id() noexcept { return {Facility::DMA_FACILITY, 1}; }

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = dma::Stm32L4DmaAccessor<1>;

  template <size_t NUM_ISRS>
  static constexpr void update_vector_table(config::VectorTable<NUM_ISRS>& table,
                                            const configuration& config) noexcept {
    // TODO(james): Move these to the device definitions for the DMA channels.
    table.add_arm_isr(DMA2_Channel1_IRQn, dma::handle_interrupt<1, 0>);
    table.add_arm_isr(DMA2_Channel2_IRQn, dma::handle_interrupt<1, 1>);
    table.add_arm_isr(DMA2_Channel3_IRQn, dma::handle_interrupt<1, 2>);
    table.add_arm_isr(DMA2_Channel4_IRQn, dma::handle_interrupt<1, 3>);
    table.add_arm_isr(DMA2_Channel5_IRQn, dma::handle_interrupt<1, 4>);
    table.add_arm_isr(DMA2_Channel6_IRQn, dma::handle_interrupt<1, 5>);
    table.add_arm_isr(DMA2_Channel7_IRQn, dma::handle_interrupt<1, 6>);
  }
};

/**
 * Configuration for the two analog to digital converters (ADC).
 */
template <>
struct DeviceDefinition<Facility::ADC_FACILITY, 0> final : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "adc1"; }

  static constexpr DeviceId id() noexcept { return {Facility::ADC_FACILITY, 0}; }

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = adc::Stm32L4AdcAccessor<System, DeviceRegistry, /* ADC1 */ 0, /* DMA1 */ 0,
                                           /* Channel 1 */ 0>;

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},  //
        DeviceId{Facility::DMA_FACILITY, 0}   //
    };
  }
};

template <>
struct DeviceDefinition<Facility::ADC_FACILITY, 1> final : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "adc2"; }

  static constexpr DeviceId id() noexcept { return {Facility::ADC_FACILITY, 1}; }

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor = adc::Stm32L4AdcAccessor<System, DeviceRegistry, /* ADC2 */ 1, /* DMA1 */ 0,
                                           /* Channel 2 */ 1>;

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},  //
        DeviceId{Facility::DMA_FACILITY, 0}   //
    };
  }
};

/**
 * Configuration for the board identification facility.
 */
template <>
struct DeviceDefinition<Facility::BOARD_IDENTIFICATION_FACILITY, 0> final
    : public config::UnconfigurableDevice {
  static constexpr const char* name() noexcept { return "board_id"; }

  static constexpr DeviceId id() noexcept { return {Facility::BOARD_IDENTIFICATION_FACILITY, 0}; }

  static constexpr gpio::PinRef BOARD_ID_POWER_PIN{/*GPIO PORT A */ 0, 6};
  static constexpr gpio::PinRef BOARD_ID_SENSE_PIN{/*GPIO PORT A */ 0, 7};

  template <typename System, typename DeviceRegistry, configuration CONFIG>
  using accessor =
      board_identification::BoardIdentificationAccessor<System, DeviceRegistry, /* ADC 0 */ 0,
                                                        BOARD_ID_POWER_PIN, BOARD_ID_SENSE_PIN>;

  static constexpr auto dependencies(const configuration&) noexcept {
    return std::array{
        DeviceId{Facility::RCC_FACILITY, 0},                         //
        DeviceId{Facility::ADC_FACILITY, 0},                         //
        DeviceId{Facility::GPIO_FACILITY, BOARD_ID_POWER_PIN.port},  //
        DeviceId{Facility::GPIO_FACILITY, BOARD_ID_SENSE_PIN.port}   //
    };
  }
};

}  // namespace internal

struct Board final {
  template <Facility FACILITY, size_t INDEX>
  using devices = internal::DeviceDefinition<FACILITY, INDEX>;
};

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::RCC_FACILITY, 0>, Board>);
static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::OSCILLATOR_FACILITY, 1>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::OSCILLATOR_FACILITY, 2>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::SYSTICK_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::GPIO_FACILITY, 0>, Board>);
static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::GPIO_FACILITY, 7>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::WATCHDOG_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::RNG_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::TIMER_FACILITY, 0>, Board>);
static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::TIMER_FACILITY, 1>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::LED_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::CLOCK_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::DMA_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::DMA_FACILITY, 1>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::ADC_FACILITY, 0>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::ADC_FACILITY, 1>, Board>);

static_assert(config::ValidDeviceDefinition<
              typename Board::template devices<Facility::BOARD_IDENTIFICATION_FACILITY, 0>, Board>);

}  // namespace tvsc::hal2::board
