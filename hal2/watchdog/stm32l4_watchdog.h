#pragma once

#include "hal2/rcc/oscillator.h"
#include "hal2/register.h"
#include "hal2/watchdog/watchdog.h"
#include "third_party/stm32/stm32.h"
#include "third_party/stm32/stm32_hal.h"

namespace tvsc::hal2::watchdog {

class IwdgRegisterBank final {
 public:
  // Key Register used for unlocking write protection, reloading the down-counter,
  // and starting the independent watchdog.
  // Offset 0x00
  volatile Register KR;

  // Prescaler Register used to configure the clock divider for the low-speed internal oscillator.
  // Offset 0x04
  volatile Register PR;

  // Reload Register used to specify the initial down-counter value that dictates the timeout
  // period.
  // Offset 0x08
  volatile Register RLR;

  // Status Register used to monitor asynchronous clock-domain synchronization flags for prescaler,
  // reload, and window updates.
  // Offset 0x0c
  volatile Register SR;

  // Window Register used to define the upper threshold boundary for windowed watchdog refresh
  // constraints.
  // Offset 0x10
  volatile Register WINR;
};

class Stm32L4Watchdog final : public WatchdogPeripheral {
 private:
  IwdgRegisterBank* registers_;
  rcc::Oscillator* lsi_oscillator_;
  // Create an activation instance, but it is invalid by default.
  rcc::OscillatorDriver lsi_active_{};

  void enable() noexcept override;
  void disable() noexcept override;

  void feed() noexcept override;

 public:
  Stm32L4Watchdog(rcc::Oscillator& lsi_oscillator) noexcept
      : registers_(reinterpret_cast<IwdgRegisterBank*>(IWDG)), lsi_oscillator_(&lsi_oscillator) {}

  [[nodiscard]] std::chrono::milliseconds reset_interval() noexcept override;
};

}  // namespace tvsc::hal2::watchdog
