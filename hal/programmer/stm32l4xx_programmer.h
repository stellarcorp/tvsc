#pragma once

#include <chrono>
#include <cstdint>

#include "bits/bits.h"
#include "hal/gpio/gpio.h"
#include "hal/programmer/programmer.h"

namespace tvsc::hal::programmer {

/**
 * Overall timing strategy:
 *
 * - Each primitive ends with a transition of SWCLK to low (a falling clock edge). Equivalently, we
 * start each primitive with the clock low and need to hold it for there for half of the clock
 * period.
 * - Output primitives begin by driving SWDIO at the start of the primitive.
 * - Input primitives begin by holding the clock low for half the period. SWDIO is then read before
 * SWCLK is driven high.
 * - The mode of SWDIO is not guaranteed at the start of a primitive. Each primitive must put SWDIO
 * in the mode correct for the primitive's operation.
 */
class ProgrammerStm32l4xx final : public ProgrammerPeripheral {
 private:
  gpio::GpioPeripheral* gpio_peripheral_;
  gpio::Gpio gpio_{};
  gpio::Pin swdio_pin_;
  gpio::Pin swclk_pin_;
  gpio::Pin reset_pin_;
  uint32_t half_clock_period_cycles_{};
  SwdioDriveState current_swdio_drive_state_{SwdioDriveState::FLOAT};

  static constexpr gpio::PinMode swdio_output_mode_{gpio::PinMode::OUTPUT_OPEN_DRAIN_WITH_PULL_UP};
  static constexpr gpio::PinMode swdio_input_mode_{gpio::PinMode::INPUT_WITH_PULL_UP};

  __attribute__((always_inline)) void half_period_delay() {
    // TODO(james): Use the embedded clock here, instead of reinventing something.
    for (volatile uint32_t j = half_clock_period_cycles_; j--; /* Update in condition */) {
      asm("nop");
    }
  }

  void enable() override;
  void disable() override;

  void initiate_target_board_reset() override;
  void conclude_target_board_reset() override;

  void set_clock_period(std::chrono::nanoseconds clock_period) override;

  void turnaround(SwdioDriveState state) override;

  void send_no_parity(uint32_t data, uint8_t bits_to_send) override;
  void send(uint32_t data, uint8_t bits_to_send) override;

  void receive_no_parity(uint32_t& data, uint8_t bits_to_receive) override;
  bool receive(uint32_t& data, uint8_t bits_to_receive) override;

 public:
  ProgrammerStm32l4xx(gpio::GpioPeripheral& gpio_peripheral, gpio::Pin swdio_pin,
                      gpio::Pin swclk_pin, gpio::Pin reset_pin)
      : gpio_peripheral_(&gpio_peripheral),
        swdio_pin_(swdio_pin),
        swclk_pin_(swclk_pin),
        reset_pin_(reset_pin) {}
};

}  // namespace tvsc::hal::programmer
