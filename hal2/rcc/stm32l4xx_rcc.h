#pragma once

#include <cstddef>
#include <cstdint>

#include "hal2/rcc/oscillator.h"
#include "hal2/rcc/rcc.h"

extern "C" {

/**
 * These symbols are usually required by the ARM CMSIS code. They are typically defined in a file
 * named system_<device>.c. We declare them here so that they can be used in our code as well; if
 * we're paying for them, we're using them.
 *
 * It's not clear what is required of these symbols or why they are left to user code to implement
 * and manage. I suspect they were originally intended to be hooks to allow for configurability,
 * like the main() function, but instead, they became requirements of the interface. With that
 * mindset, these functions should be assumed to be the most minimal implementation that makes the
 * CPU work, and all other functionality should be available via other abstractions.
 *
 * The best documentation I can find about these symbols is
 * https://arm-software.github.io/CMSIS_6/latest/Core/group__system__init__gr.html
 *
 * Currently, the implementation being used is from ST Micro.
 */

/*
 * SystemCoreClock is a global variable required by the ARM CMSIS and STM32 HAL. It contains the
 * core clock value in ticks per second.
 *
 * For more information, see
 * https://arm-software.github.io/CMSIS_5/Core/html/group__system__init__gr.html#gaa3cd3e43291e81e795d642b79b6088e6
 */
extern uint32_t SystemCoreClock;
}

namespace tvsc::hal2::rcc {

/**
 * Class to manage the reset and clock circuitry (RCC) on the Nucleo L452RE board and other boards
 * based on the STM32L4xx series of CPUs.
 */
class RccStm32L4xx final : public Rcc {
 private:
  enum class ClockConfiguration {
    UNCONFIGURED,
    MIN_SPEED,
    MAX_SPEED,
    // The energy efficient speed attempts to minimize the total energy required to execute a
    // CPU-intensive task.
    ENERGY_EFFICIENT_SPEED,
  };
  ClockConfiguration clock_configuration_{ClockConfiguration::UNCONFIGURED};

 public:
  RccStm32L4xx() { set_clock_to_energy_efficient_speed(); }
  void set_clock_to_max_speed() override;
  void set_clock_to_min_speed() override;
  void set_clock_to_energy_efficient_speed() override;
  void restore_clock_speed() override;

  /**
   * Before entering sleep or stop mode, be sure to configure appropriate interrupts. Otherwise, you
   * may need to halt the processor via a debugging tool to regain control.
   */
  void enter_sleep_mode() override;
  void enter_stop_mode() override;
};

class LsiOscillatorStm32L4xx final : public Oscillator {
 private:
  void enable() override;
  void disable() override;
};

/**
 * Manage the oscillator for USB, RNG, and other functions that require a precise, (relatively) high
 * speed oscillator.
 *
 * In the STM32L4xx series, this oscillator can only be used as the clock for USB and RNG. It cannot
 * be used as a generic SYSCLK or drive any other peripherals. This limitation makes it ideal for
 * clocking those two peripherals. We do not need to disturb the SYSCLK or the clocks of other
 * peripherals.
 *
 * This oscillator is likely specific to STM32 processors, but it's unclear how this functionality
 * could be abstracted better at this time. Somehow, we would need to include the notion of multiple
 * oscillators, model which oscillators can provide clock signals for which sets of peripherals, and
 * manage routing those oscillators dynamically. Rather than tackle this problem, we have punted on
 * it completely and simply named this oscillator as it is named in the STM32 MCU documentation.
 */
class Hsi48OscillatorStm32L4xx final : public Oscillator {
 private:
  void enable() override;
  void disable() override;
};

}  // namespace tvsc::hal2::rcc
