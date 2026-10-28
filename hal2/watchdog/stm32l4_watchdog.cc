#include "hal2/watchdog/stm32l4_watchdog.h"

#include <chrono>
#include <cstdlib>

#include "hal2/error.h"

namespace tvsc::hal2::watchdog {

using namespace std::chrono_literals;
static constexpr std::chrono::milliseconds RESET_INTERVAL{2s};

struct CounterConfig final {
  uint32_t prescale_value;
  uint32_t counter_value;
};

void Stm32L4Watchdog::feed() {
  // Well-known value required to prevent a reset.
  static constexpr uint32_t FEED_KEY_VALUE{0xaaaa};
  registers_->KR.set_value(FEED_KEY_VALUE);
}

std::chrono::milliseconds Stm32L4Watchdog::reset_interval() noexcept { return RESET_INTERVAL; }

void Stm32L4Watchdog::enable() noexcept {
  static constexpr uint32_t LSI_FREQ_HZ{32000};

  static constexpr uint32_t WINDOW_DISABLE_VALUE{0x0fff};
  static constexpr uint32_t MAX_PRESCALE_VALUE{0b111};
  static constexpr uint32_t MAX_RELOAD_VALUE{(1UL << 12) - 1};
  static constexpr uint32_t CONFIGURE_KEY_VALUE{0x5555};
  static constexpr uint32_t START_KEY_VALUE{0xcccc};

  static constexpr CounterConfig counter_configuration = []() constexpr -> CounterConfig {
    auto interval{std::chrono::duration_cast<std::chrono::milliseconds>(RESET_INTERVAL)};
    uint64_t tick_count{static_cast<uint64_t>((LSI_FREQ_HZ * interval / 1'000).count())};

    uint64_t divider{4};
    uint32_t prescale_value{0};
    while (prescale_value <= MAX_PRESCALE_VALUE) {
      if ((tick_count / divider) <= MAX_RELOAD_VALUE) {
        break;
      }
      ++prescale_value;
      divider *= 2;
    }
    return {prescale_value, static_cast<uint32_t>(tick_count / divider)};
  }();
  static_assert(counter_configuration.prescale_value <= MAX_PRESCALE_VALUE,
                "RESET_INTERVAL is too long and will overflow register limits.");
  static_assert(counter_configuration.counter_value <= MAX_RELOAD_VALUE,
                "RESET_INTERVAL is too long and will overflow register limits.");

  // Ensure the LSI oscillator is on. Note that this step is technically not required. Enabling the
  // IWDG automatically turns on the LSI oscillator. We do this extra step, because it ensures that
  // the refcount on the LSI oscillator peripheral in sync with the hardware's behavior.
  lsi_active_ = lsi_oscillator_->access();

  // NOTE: if ever this code transitions to allow the watchdog to be reconfigured after it has been
  // started, be sure to feed the watchdog before this configuration sequence. Otherwise, the
  // watchdog may be reaching the end of its counter and may reset the MCU in the process of a
  // configuration change.
  // registers_->KR = FEED_KEY_VALUE;

  // Unlock watchdog for configuration.
  registers_->KR.set_value(CONFIGURE_KEY_VALUE);

  // Update the prescaler
  while (registers_->SR.bit_field_value<1, 0>() != 0) {
    // Block until prescaler is ready to be updated.
  }
  registers_->PR.set_value(counter_configuration.prescale_value);

  // Update the reload value
  while (registers_->SR.bit_field_value<1, 1>() != 0) {
    // Block until reload value register is ready to be updated.
  }
  registers_->RLR.set_value(counter_configuration.counter_value);

  // Update the window value
  while (registers_->SR.bit_field_value<1, 2>() != 0) {
    // Block until the window value register is ready to be updated.
  }
  registers_->WINR.set_value(WINDOW_DISABLE_VALUE);

  // Wait for all updates to be synchronized across clock domains.
  while (registers_->SR.value() != 0) {
    // Block until ready.
  }

  // Start the watchdog.
  registers_->KR.set_value(START_KEY_VALUE);
}

void Stm32L4Watchdog::disable() noexcept {
  // Watchdogs can't be disabled by design, except on system reset.
  // Attempting to disable the watchdog means that we are no longer actively feeding it. In that
  // case, it is best to fail sooner, rather than wait for the watchdog to reset the system.
  error();
}

}  // namespace tvsc::hal2::watchdog
