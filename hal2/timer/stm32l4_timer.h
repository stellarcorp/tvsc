#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>

#include "base/except.h"
#include "hal2/rcc/oscillator.h"
#include "hal2/register.h"
#include "hal2/timer/timer.h"
#include "platforms/linker_sections.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::timer {

DEBUG_VAR volatile int interrupt_fired{};

class LptimRegisterBank final {
 public:
  // Holds flag bits indicating various timer events such as compare matches, autoreload updates,
  // and counter direction changes.
  // Offset 0x00
  volatile Register ISR;

  // Clears specific flags in the interrupt and status register when written with high bits.
  // Offset 0x04
  volatile Register ICR;

  // Controls which timer events are permitted to trigger hardware interrupts.
  // Offset 0x08
  volatile Register IER;

  // Configures operational parameters including clock source selection, prescaling, filter
  // settings, and trigger polarities.
  // Offset 0x0c
  volatile Register CFGR;

  // Controls peripheral activation, software triggers, and starts the timer in single or continuous
  // counting modes.
  // Offset 0x10
  volatile Register CR;

  // Holds the threshold value compared against the running counter to trigger match events or
  // output changes.
  // Offset 0x14
  volatile Register CMP;

  // Defines the upper limit or period of the counter, determining when it resets or rolls over.
  // Offset 0x18
  volatile Register ARR;

  // Exposes the current active tick count of the low power timer.
  // Offset 0x1c
  volatile Register CNT;
};

class Stm32l4xxLptim final : public TimerPeripheral {
 private:
  volatile LptimRegisterBank* registers_;
  rcc::Oscillator* lsi_oscillator_;
  rcc::OscillatorDriver lsi_driver_{};

  // Returns a volatile reference to the underlying register bank mapped via placement new.
  static volatile LptimRegisterBank* instantiate_registers(uint8_t TIMER_INDEX) noexcept {
    void* const base_address{(TIMER_INDEX == 0) ? LPTIM1 : LPTIM2};
    return reinterpret_cast<volatile LptimRegisterBank*>(base_address);
  }

  uint8_t timer_index() const noexcept {
    if ((void*)registers_ == (void*)LPTIM1) {
      return 0;
    } else if ((void*)registers_ == (void*)LPTIM2) {
      return 1;
    } else {
      except<std::logic_error>("Unknown LPTIM instance.");
    }
  }

 public:
  explicit Stm32l4xxLptim(uint8_t TIMER_INDEX, rcc::Oscillator& lsi_oscillator) noexcept
      : registers_(instantiate_registers(TIMER_INDEX)), lsi_oscillator_(&lsi_oscillator) {
    if (TIMER_INDEX == 0) {
      HAL_NVIC_SetPriority(LPTIM1_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
    } else if (TIMER_INDEX == 1) {
      HAL_NVIC_SetPriority(LPTIM2_IRQn, 6, 0);
      HAL_NVIC_EnableIRQ(LPTIM2_IRQn);
    }
  }

  void configure_single_pulse(uint32_t interval_us) noexcept override {
    static constexpr uint32_t lsi_freq_hz{32000};

    uint64_t timeout_ticks{static_cast<uint64_t>(lsi_freq_hz) * interval_us / 1'000'000UL};
    uint32_t prescaler_value{0};

    if (timeout_ticks < (1UL << 16)) {
      prescaler_value = 0;
    } else if (timeout_ticks < (1UL << 17)) {
      prescaler_value = 1;
      timeout_ticks /= 2;
    } else if (timeout_ticks < (1UL << 18)) {
      prescaler_value = 2;
      timeout_ticks /= 4;
    } else if (timeout_ticks < (1UL << 19)) {
      prescaler_value = 3;
      timeout_ticks /= 8;
    } else if (timeout_ticks < (1UL << 20)) {
      prescaler_value = 4;
      timeout_ticks /= 16;
    } else if (timeout_ticks < (1UL << 21)) {
      prescaler_value = 5;
      timeout_ticks /= 32;
    } else if (timeout_ticks < (1UL << 22)) {
      prescaler_value = 6;
      timeout_ticks /= 64;
    } else {
      prescaler_value = 7;
      timeout_ticks /= 128;
    }
    timeout_ticks =
        std::clamp(timeout_ticks, static_cast<uint64_t>(128), static_cast<uint64_t>(0xffffUL));

    // Ensure the timer is disabled before modifying configuration registers.
    registers_->CR.set_value(0);

    // Configure clock prescaler in the configuration register (PRESC is bits 11:9, offset 9).
    registers_->CFGR.set_bit_field_value<1, 0>(0x0);  // Use an internal clock source.
    registers_->CFGR.set_bit_field_value<2, 6>(0x0);  // All trigger level changes are valid.
    registers_->CFGR.set_bit_field_value<3, 9>(prescaler_value);
    registers_->CFGR.set_bit_field_value<2, 17>(0x0);  // Internal (software) starts the counting.
    registers_->CFGR.set_bit_field_value<1, 22>(0x0);  // Update the registers now using core clock.
    registers_->CFGR.set_bit_field_value<1, 23>(0x0);  // Increment after each internal clock pulse.
    registers_->CFGR.set_bit_field_value<1, 24>(0x0);  // Disable encoder mode.

    // Disable all interrupts while we configure the peripheral.
    registers_->IER.set_value(0);

    // Enable the timer.
    // The CPU writes register values using the system bus clock, while the timer runs on its own
    // independent kernel clock. Because these two clocks operate independently, writes to ARR and
    // CMP must cross from the bus domain into the timer domain. Hardware synchronizer circuits
    // manage this cross-domain transfer. These synchronizers require active clock edges from the
    // timer's kernel clock to function. Consequently, the peripheral must be enabled before the
    // ARR, CMP, and IER registers are configured so that its clock is running to latch the new
    // values.
    registers_->CR.set_value(1);

    // Clear all interrupts.
    registers_->ICR.set_value(0x7f);

    // Write the new period to ARR.
    registers_->ARR.set_value(static_cast<uint32_t>(timeout_ticks));

    // Wait for the hardware to confirm the write has synchronized.
    while (registers_->ISR.bit_field_value<1, 4>() == 0) {
      // Optional: Add a timeout counter here to prevent infinite hangs if hardware faults.
    }
    // Clear the autoreload register update flag.
    registers_->ICR.set_value(0x7f);

    // Write the pulse value to CMP.
    registers_->CMP.set_value(static_cast<uint32_t>(timeout_ticks));

    // Wait for the compare register write to synchronize.
    while (registers_->ISR.bit_field_value<1, 3>() == 0) {
      // Optional timeout check.
    }
    // Clear the compare update flag.
    registers_->ICR.set_value(0x7f);

    // Enable the compare match interrupt (CMPMIE) and disable all of the other interrupts.
    registers_->IER.set_value(1);
  }

  void start() noexcept override { registers_->CR.set_value(0x03); }

  void stop() noexcept override {
    registers_->ICR.set_value(0x7f);
    registers_->CR.set_value(0);
  }

  [[nodiscard]] bool is_running() noexcept override {
    // Check the enable bit in the control register. If it is on, assume that the timer is running.
    return registers_->CR.bit_field_value<1, 0>() == 1;
  }

  void enable() noexcept override {
    lsi_driver_ = lsi_oscillator_->access();

    if (timer_index() == 0) {
      __HAL_RCC_LPTIM1_CLK_ENABLE();
    } else if (timer_index() == 1) {
      __HAL_RCC_LPTIM2_CLK_ENABLE();
    }

    lsi_driver_.route_to(config::Facility::TIMER_FACILITY, timer_index());
  }

  void disable() noexcept override {
    if (timer_index() == 0) {
      __HAL_RCC_LPTIM1_CLK_DISABLE();
    } else if (timer_index() == 1) {
      __HAL_RCC_LPTIM2_CLK_DISABLE();
    }

    lsi_driver_.invalidate();
  }
};

void handle_low_power_timer1_interrupt() noexcept {
  ++interrupt_fired;
  // Overlay register bank via placement new to clear pending interrupt flags directly.
  volatile LptimRegisterBank* lptim_registers{
      reinterpret_cast<volatile LptimRegisterBank*>(LPTIM1)};
  if (lptim_registers->ISR.bit_field_value<1, 1>() == 1) {
    // Clear the autoreload match interrupt.
    lptim_registers->ICR.set_value(0x7f);
    // Disable the timer.
    lptim_registers->CR.set_bit_field_value<1, 0>(0);
  }
  if (lptim_registers->ISR.bit_field_value<1, 0>() == 1) {
    // Clear the compare match interrupt.
    lptim_registers->ICR.set_value(0x7f);
  }
  lptim_registers->ICR.set_value(0x7f);
}

void handle_low_power_timer2_interrupt() noexcept {
  ++interrupt_fired;
  // Overlay register bank via placement new to clear pending interrupt flags directly.
  volatile LptimRegisterBank* lptim_registers{
      reinterpret_cast<volatile LptimRegisterBank*>(LPTIM2)};
  if (lptim_registers->ISR.bit_field_value<1, 1>() == 1) {
    // Clear the autoreload match interrupt.
    lptim_registers->ICR.set_value(0x7f);
    // Disable the timer.
    lptim_registers->CR.set_bit_field_value<1, 0>(0);
  }
  if (lptim_registers->ISR.bit_field_value<1, 0>() == 1) {
    // Clear the compare match interrupt.
    lptim_registers->ICR.set_value(0x7f);
  }
  lptim_registers->ICR.set_value(0x7f);
}

}  // namespace tvsc::hal2::timer
