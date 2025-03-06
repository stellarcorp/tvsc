#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "hal/error.h"
#include "hal/gpio/gpio.h"
#include "hal/gpio/gpio_interceptor.h"
#include "hal/gpio/gpio_noop.h"
#include "hal/power/power.h"
#include "hal/power/power_interceptor.h"
#include "hal/power/power_noop.h"
#include "hal/rcc/rcc.h"
#include "hal/rcc/rcc_interceptor.h"
#include "hal/rcc/rcc_noop.h"
#include "hal/simulation/reactor.h"
#include "hal/systick/fake_systick.h"
#include "hal/systick/systick.h"
#include "hal/systick/systick_interceptor.h"
#include "hal/systick/systick_noop.h"
#include "hal/timer/timer.h"
#include "hal/timer/timer_interceptor.h"
#include "hal/timer/timer_noop.h"
#include "hal/watchdog/watchdog.h"
#include "hal/watchdog/watchdog_interceptor.h"
#include "hal/watchdog/watchdog_noop.h"
#include "time/scaled_clock.h"

namespace tvsc::hal::board {

class Board final {
 public:
  static constexpr gpio::Port NUM_GPIO_PORTS{1};
  static constexpr size_t NUM_DAC_CHANNELS{0};
  static constexpr size_t NUM_DEBUG_LEDS{1};

  static constexpr gpio::Port GPIO_PORT_A{0};

  // Location of the LEDs provided by this board.
  static constexpr gpio::Port GREEN_LED_PORT{GPIO_PORT_A};
  static constexpr gpio::Pin GREEN_LED_PIN{5};

  static constexpr std::array<gpio::Port, NUM_DEBUG_LEDS> DEBUG_LED_PORTS{GREEN_LED_PORT};
  static constexpr std::array<gpio::Pin, NUM_DEBUG_LEDS> DEBUG_LED_PINS{GREEN_LED_PIN};

  // We use a simulation clock that gives one millisecond of time in the simulation for every
  // microsecond on the running system's steady_clock. This ratio simulates a much slower CPU than
  // the one running the simulation. The exact ratio is not exactly 1000x as given here and would
  // vary depending on many factors -- configured speed of the simulated CPU, clock accuracies,
  // timings of entering and exiting sleep and stop modes, thermal throttling of simulation system,
  // load of simulation system, etc. Achieving an exact timing for the simulated CPU is out of
  // scope.
  using SimulationClockType = time::ScaledClock<1000, 1, std::chrono::steady_clock>;

 private:
  simulation::Reactor<SimulationClockType> reactor_{SimulationClockType::clock()};

  rcc::RccNoop rcc_{};
  rcc::RccInterceptor rcc_interceptor_{rcc_};

  systick::FakeSysTick<SimulationClockType> systick_{reactor_};
  systick::SysTickInterceptor systick_interceptor_{systick_};

  // We initialize these GPIO ports with the addresses where their registers are bound.
  // Note that the STM32L4xx boards seem to have up to 11 (A-K) GPIO ports. We have only provided
  // for the first few here, but this can be expanded if necessary.
  gpio::GpioNoop gpio_{};
  gpio::GpioInterceptor gpio_interceptor_{gpio_};
  // Don't forget to modify NUM_GPIO_PORTS and add a GPIO_PORT_* above.

  power::PowerNoop power_{};
  power::PowerInterceptor power_interceptor_{power_};

  timer::TimerNoop timer_{};
  timer::TimerInterceptor timer_interceptor_{timer_};

  watchdog::WatchdogNoop iwdg_{};
  watchdog::WatchdogNoop iwdg_interceptor_{iwdg_};

  static Board board_;

  // Private constructor to restrict inadvertent instantiation and copying.
  Board();

 public:
  // One board per executable.
  static Board& board();

  template <gpio::Port GPIO_PORT>
  gpio::GpioPeripheral& gpio() {
    static_assert(
        GPIO_PORT < NUM_GPIO_PORTS,
        "Invalid GPIO port id. Likely, there is a mismatch in the build that instantiates a Board "
        "without considering the correct BOARD_ID. Verify that the board-specific header file "
        "(hal/boards/board_<board-name>.h) is being included.");
    if constexpr (GPIO_PORT == 0) {
      return gpio_interceptor_;
    }
  }

  /**
   * Accessor for GPIO periperhals. Note that the templated version above is vastly preferred, as it
   * gives compile-time errors, as opposed to runtime errors like this method. This method should
   * only be used as to lookup GPIO ports using integer values that can be evaluated at
   * compile-time; these integer values will mainly come from STM's HAL.
   */
  gpio::GpioPeripheral& gpio(gpio::Port port) {
    if (port == 0) {
      return gpio_interceptor_;
    }
    error();
  }

  watchdog::WatchdogPeripheral& iwdg() { return iwdg_interceptor_; }
  power::Power& power() { return power_interceptor_; };
  rcc::Rcc& rcc() { return rcc_interceptor_; };
  timer::TimerPeripheral& sleep_timer() { return timer_interceptor_; }
  systick::SysTickType& sys_tick() { return systick_interceptor_; }
};

}  // namespace tvsc::hal::board
