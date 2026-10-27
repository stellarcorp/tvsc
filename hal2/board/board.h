#pragma once

#include <cstddef>

#include "hal2/config/facility.h"

namespace tvsc::hal2::board::internal {

template <config::Facility FACILITY, size_t INDEX>
struct PeripheralConfiguration;

}  // namespace tvsc::hal2::board::internal

#if defined(CHEERFUL_COFFEE)
#include "hal2/board/cheerful_coffee_board.h"
#else
#error \
    "Please configure this file to include the appropriate symbol (NUCLEO_L412KB, etc.) for the platform. Alternatively, update .bazelrc to define the appropriate symbol using --copt."
#endif

namespace tvsc::hal2::board {

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
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::TIMER_FACILITY, 0>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::TIMER_FACILITY, 1>, Board>);
static_assert(config::ValidPeripheralConfiguration<
              internal::PeripheralConfiguration<config::Facility::CLOCK_FACILITY, 0>, Board>);

}  // namespace tvsc::hal2::board
